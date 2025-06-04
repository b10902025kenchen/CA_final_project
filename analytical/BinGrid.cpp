#include "BinGrid.h"

double ovlp_area(Star &star, Bin &bin) {
    double x1 = max(star.x(), double(bin.l_x));
    double y1 = max(star.y(), double(bin.l_y));
    double x2 = min(star.x() + star.width(), double(bin.u_x));
    double y2 = min(star.y() + star.height(), double(bin.u_y));
    return max(0.0, x2 - x1) * max(0.0, y2 - y1);
}

BinGrid::BinGrid(double x0, double x1, double y0, double y1, vector<Star*> stars)
{
    cout<<"BinGrid constructor called"<<endl;
    _pModules.resize(stars.size());
    for(int i = 0 ; i < stars.size() ; i++)
    {
        _pModules[i] = stars[i];
    }

    len_x = (x1 - x0);
    len_y = (y1 - y0);
    lower_x = x0;
    lower_y = y0;

    _num_bins_x = 1<<int(ceil(log2(len_x)));
    _num_bins_y = 1<<int(ceil(log2(len_y))+1);


    _Bin2D.resize(_num_bins_x, vector<Bin>(_num_bins_y));

    
    _field_cache.resize(stars.size(), Point2<double>(0,0));

    // interface with FFT LIBRARY UPSCALE THE 2D FACTOR INITIALLY
    density_map = new float*[_num_bins_x];
    for(int i = 0 ; i < _num_bins_x ; i++)
        density_map[i] = new float[_num_bins_y];
    electroPhi_ = new float*[_num_bins_x];
    for(int i = 0 ; i < _num_bins_x ; i++)
        electroPhi_[i] = new float[_num_bins_y];
    field_x = new float*[_num_bins_x];
    for(int i = 0 ; i < _num_bins_x ; i++)
        field_x[i] = new float[_num_bins_y];
    field_y = new float*[_num_bins_x];
    for(int i = 0 ; i < _num_bins_x ; i++)
        field_y[i] = new float[_num_bins_y];


    double die_area = len_x * len_y;
    target_density = 0.9;
    

    
    initBin2D();
}

BinGrid::~BinGrid()
{
    cout<<"BinGrid destructor called"<<endl;
    for(int i = 0 ; i < _Bin2D.size() ; i++)
    {
        delete[] density_map[i];
        delete[] electroPhi_[i];
        delete[] field_x[i];
        delete[] field_y[i];
    }
    if(_Bin2D.size() > 0)
    {
        delete[] density_map;
        delete[] electroPhi_;
        delete[] field_x;
        delete[] field_y;
    }
    density_map = nullptr;
    electroPhi_ = nullptr;
    field_x = nullptr;
    field_y = nullptr;
    cout<<"BinGrid destructor called"<<endl;
}

void BinGrid::rescale(int x_in, int y_in)
{
    _num_bins_x = x_in;
    _num_bins_y = y_in;
    initBin2D();
}

void BinGrid::initBin2D()
{

    for(int i = 0 ; i < _pModules.size() ; i++)
    {
        if(_pModules[i]->invalid) continue; // Skip invalid stars
        maximal_weight = max(maximal_weight, _pModules[i]->score);
    }
    _cos_table.resize( max(_num_bins_x, _num_bins_y) * 3 / 2, 0 );
    ip_table.resize( round(sqrt(max(_num_bins_x, _num_bins_y))) + 2, 0 );
    
    for(int i = 0 ; i < _num_bins_x ; i++)
    {
        for(int j = 0 ; j < _num_bins_y ; j++)
        {
            _Bin2D[i][j].l_x = (i * len_x) / _num_bins_x + lower_x;
            _Bin2D[i][j].l_y = (j * len_y) / _num_bins_y + lower_y;
            _Bin2D[i][j].u_x = ((i+1) * len_x) / _num_bins_x + lower_x;
            _Bin2D[i][j].u_y = ((j+1) * len_y) / _num_bins_y + lower_y;
        }
    }
    for(int i = 0 ; i < _num_bins_x ; i++)
    {
        for(int j = 0 ; j < _num_bins_y ; j++)
        {
            _Bin2D[i][j].ovlp_area = 0;
        }
    }
    
}

void BinGrid::updateBin2D() {

    // Update the 2D bin grid
    for(int i = 0 ; i < _Bin2D.size() ; i++)
    {
        for(int j = 0 ; j < _Bin2D[0].size() ; j++)
        {
            _Bin2D[i][j].ovlp_area = 0;
        }
    }


    for (unsigned i = 0; i < _pModules.size(); i++) {
        if(_pModules[i]->invalid) continue; // Skip invalid stars
        Star star = *_pModules[i];
        int l_idx_x = (star.x() - lower_x) / _Bin2D[0][0].len_x();
        int l_idx_y = (star.y() - lower_y) / _Bin2D[0][0].len_y();
        int u_idx_x = (star.x() + star.width() - lower_x) / _Bin2D[0][0].len_x();
        int u_idx_y = (star.y() + star.height() - lower_y) / _Bin2D[0][0].len_y();
        if(l_idx_x < 0) l_idx_x = 0;
        if(l_idx_y < 0) l_idx_y = 0;
        if(u_idx_x >= _num_bins_x) u_idx_x = _num_bins_x - 1;
        if(u_idx_y >= _num_bins_y) u_idx_y = _num_bins_y - 1;
        for (int j = l_idx_x; j <= u_idx_x; j++) {
            for (int k = l_idx_y; k <= u_idx_y; k++) {
                Bin &bin = _Bin2D[j][k];
                bin.ovlp_area += ovlp_area(star, bin)*star.score / maximal_weight;
            }
        }
    }
    double overall_area = 0;
    for(int i = 0 ; i < _Bin2D.size() ; i++)
    {
        for(int j = 0 ; j < _Bin2D[0].size() ; j++)
        {
            overall_area += _Bin2D[i][j].area();
        }
    }
    double avg_area = overall_area / (_Bin2D.size() * _Bin2D[0].size());
    for(int i = 0 ; i < _Bin2D.size() ; i++)
    {
        for(int j = 0 ; j < _Bin2D[0].size() ; j++)
        {
            _Bin2D[i][j].ovlp_area = (_Bin2D[i][j].ovlp_area - avg_area)/ _Bin2D[i][j].area();
            if(_Bin2D[i][j].ovlp_area < 0.5) _Bin2D[i][j].ovlp_area -= 1;
        }
    }
    

        
}

void BinGrid::updateBinPhi()
{

    /*
        for base func exp(i2\pi * (k_x *x + k_y * y))
        D^2 = -(k_x^2 + k_y^2)
        First use DCT to get the weight of each R(kx,ky)
        -(Kx^2 + Ky^2) * phi(kx,ky) = 2D-FFT(rho(kx,ky))
        phi(kx,ky) = -2D-FFT(rho(kx,ky)) / (Kx^2 + Ky^2)
    */
    #pragma omp parallel for
    for(int i = 0 ; i < _Bin2D.size() ; i++)
        for(int j = 0 ; j < _Bin2D[0].size() ; j++)
        {
            density_map[i][j] = _Bin2D[i][j].ovlp_area;
        }
    
    ddct2d(_num_bins_x, _num_bins_y, -1, density_map, 
        NULL, (int*) &ip_table[0], (float*)&_cos_table[0]);
    

    for(int i = 0; i < _num_bins_x; i++) {
        density_map[i][0] *= 0.5;
    }
        
    for(int i = 0; i < _num_bins_y; i++) {
        density_map[0][i] *= 0.5;
    }
    for(int i = 0; i < _num_bins_x; i++) {
        for(int j = 0; j < _num_bins_y; j++) {
            density_map[i][j] *= 4.0 / _num_bins_x / _num_bins_y;
        }
    }

}

void BinGrid::updateBinField()
{
    const int Ny = _Bin2D.size();       // rows
    const int Nx = _Bin2D[0].size();    // cols


    #pragma omp parallel for
    for(int i = 0; i < _num_bins_x; i++) {
        float wx_i = PI * float(i) / float(_num_bins_x);
        float wx_i2 = wx_i * wx_i;
    
        for(int j = 0; j < _num_bins_y; j++) {
          float wy_i = PI * float(j) / float(_num_bins_y);
          float wy_i2 = wy_i*wy_i;
    
          float density = density_map[i][j];
          float phi = 0;
          float electroX = 0, electroY = 0;
    
          if(i == 0 && j == 0) {
            phi = electroX = electroY = 0.0f;
          }
          else {
            phi = density / (wx_i2 + wy_i2);
            electroX = phi * wx_i;
            electroY = phi * wy_i;
          }
          electroPhi_[i][j] = phi;
          field_x[i][j] = -electroX;
          field_y[i][j] = -electroY;        //D \phi = -E
        }
      }



    //ddct2d(_num_bins_x, _num_bins_y, 1, 
    //    electroPhi_, NULL, 
    //    (int*) &ip_table[0], (float*) &_cos_table[0]);
    ddsct2d(_num_bins_x, _num_bins_y, 1, 
        field_x, NULL, 
        (int*) &ip_table[0], (float*) &_cos_table[0]);

    ddcst2d(_num_bins_x, _num_bins_y, 1, 
        field_y, NULL, 
        (int*) &ip_table[0], (float*) &_cos_table[0]);
    


}

void BinGrid::normalizeBinField()
{
    double max_field = 1;   //in case that the field is nearly convergent
    float len_coeff_x = 1;
    float len_coeff_y = 1;
    
    if(_Bin2D[0][0].len_x() > _Bin2D[0][0].len_y())
    {
        len_coeff_x = 1;
        len_coeff_y = _Bin2D[0][0].len_x()/_Bin2D[0][0].len_y();
        float norm = sqrt(len_coeff_x * len_coeff_x + len_coeff_y * len_coeff_y);
        len_coeff_x /= norm;
        len_coeff_y /= norm;
    }
    else
    {
        len_coeff_x = _Bin2D[0][0].len_y() / _Bin2D[0][0].len_x();
        len_coeff_y = 1;
        float norm = sqrt(len_coeff_x * len_coeff_x + len_coeff_y * len_coeff_y);
        len_coeff_x /= norm;
        len_coeff_y /= norm;
    }
    
    #pragma omp parallel for
    for(int i = 0 ; i < _pModules.size() ; i++)
    {
        _field_cache[i].x = 0;
        _field_cache[i].y = 0;
        pair<pair<int,int>, pair<int,int>> bin_idx = getBinIdx(_pModules[i]);
        for(int k = bin_idx.first.first ; k <= bin_idx.second.first ; k++)
        {
            for(int l = bin_idx.first.second ; l <= bin_idx.second.second ; l++)
            {
                double score_fac = _pModules[i]->score / maximal_weight;
                if(_pModules[i]->invalid)
                    score_fac = 0;
                _field_cache[i].x += field_x[k][l] * score_fac * ovlp_area(*_pModules[i], _Bin2D[k][l]) / _Bin2D[k][l].area()*len_coeff_x;
                _field_cache[i].y += field_y[k][l] * score_fac * ovlp_area(*_pModules[i], _Bin2D[k][l]) / _Bin2D[k][l].area()*len_coeff_y;
            }
        }
    }

    for(int i = 0 ; i < _pModules.size() ; i++)
        _field_cache[i] /= _pModules[i]->area()*_pModules[i]->score;    //precondition
    
}

vector<Point2<double>> BinGrid::getBinField()
{

    updateBin2D();

    updateBinPhi();

    updateBinField();

    normalizeBinField();

    update_ovfl();

    return _field_cache;
}   

void BinGrid::update_ovfl()
{
    ovfl = 0;
    #pragma omp parallel for
    for(int i = 0 ; i < _num_bins_x/2 ; i++)
    {
        for(int j = 0 ; j < _num_bins_y/2 ; j++)
        {
            double area = _Bin2D[2*i][2*j].ovlp_area + 
                _Bin2D[2*i+1][2*j].ovlp_area +
                _Bin2D[2*i][2*j+1].ovlp_area +
                _Bin2D[2*i+1][2*j+1].ovlp_area;
            double new_ovfl = (_Bin2D[i][j].ovlp_area - target_density) / target_density;
            
            if(new_ovfl > ovfl)
                ovfl = new_ovfl;  
        }
    }
}

double BinGrid::get_energy()
{
    double energy = 0;
    for(int i = 0 ; i < _pModules.size() ; i++)
    {
        pair<pair<int,int>, pair<int,int>> bin_idx = getBinIdx(_pModules[i]);
        for(int k = bin_idx.first.first ; k <= bin_idx.second.first ; k++)
        {
            for(int l = bin_idx.first.second ; l <= bin_idx.second.second ; l++)
            {
                energy += electroPhi_[k][l] * ovlp_area(*_pModules[i], _Bin2D[k][l]) / _Bin2D[k][l].area();
            }
        }
    }
    return energy;
}

void BinGrid::apply2DFFT(vector<vector<cd>>& a, bool invert = false)
{
    const int Ny = a.size();
    const int Nx = a[0].size();


    for (int y = 0; y < Ny; ++y) {
        std::vector<cd> tmp;
        if(!invert)
            dct1d(a[y], tmp);
        else
            idct1d(a[y], tmp);
        a[y].swap(tmp);
    }

    for (int x = 0; x < Nx; ++x) {
        std::vector<cd> col(Ny);
        for (int y=0; y<Ny; ++y) col[y] = a[y][x];
        std::vector<cd> tmp;
        if(!invert)
            dct1d(col, tmp);
        else
            idct1d(col, tmp);
        for (int y=0; y<Ny; ++y) a[y][x] = tmp[y];
    }
}

void BinGrid::apply2DDSCT(vector<vector<cd>>& a, bool invert = false)
{
    const int Ny = a.size();
    const int Nx = a[0].size();

    for (int y = 0; y < Ny; ++y) {
        std::vector<cd> tmp;
        if(!invert)
            dst1d(a[y], tmp);
        else
            idst1d(a[y], tmp);
        a[y].swap(tmp);
    }

    for (int x = 0; x < Nx; ++x) {
        std::vector<cd> col(Ny);
        for (int y=0; y<Ny; ++y) col[y] = a[y][x];
        std::vector<cd> tmp;
        if(!invert)
            dct1d(col, tmp);
        else
            dct1d(col, tmp);
        for (int y=0; y<Ny; ++y) a[y][x] = tmp[y];
    }
}

void BinGrid::apply2DDCST(vector<vector<cd>>& a, bool invert = false)
{
    const int Ny = a.size();
    const int Nx = a[0].size();

    for (int y = 0; y < Ny; ++y) {
        std::vector<cd> tmp;
        if(!invert)
            dct1d(a[y], tmp);
        else
            idct1d(a[y], tmp);
        a[y].swap(tmp);
    }

    for (int x = 0; x < Nx; ++x) {
        std::vector<cd> col(Ny);
        for (int y=0; y<Ny; ++y) col[y] = a[y][x];
        std::vector<cd> tmp;
        if(!invert)
            dst1d(col, tmp);
        else
            dst1d(col, tmp);
        for (int y=0; y<Ny; ++y) a[y][x] = tmp[y];
    }
}

void BinGrid::fft(std::vector<cd>& a, bool invert = false)  
{
    const int n = static_cast<int>(a.size());

    // --- 1. bit‑reversal permutation ---------------------------------------
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }

    // --- 2. butterfly -------------------------------------------------------
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * (invert ? -1 : 1);
        cd wlen(std::cos(ang), std::sin(ang));

        for (int i = 0; i < n; i += len) {
            cd w(1);
            for (int j = 0; j < len / 2; ++j) {
                cd u = a[i + j];              // 上半部
                cd v = a[i + j + len / 2] * w; // 下半部 × twiddle factor
                a[i + j]            = u + v;
                a[i + j + len / 2]  = u - v;
                w *= wlen;
            }
        }
    }

    // --- 3. 若為 IFFT，需整體除以 n ----------------------------------------
    if (invert) {
        for (cd& x : a) x /= n;
    }
}

void BinGrid::dct1d(const std::vector<cd>& in,
    std::vector<cd>&       out)
{
    const int N = static_cast<int>(in.size());
    std::vector<cd> buf(2 * N);           // 長度 2N 的複數暫存

    /* 1. 建立偶對稱序列  x_e(n) = {in, 反轉(in)} ---------------------- */
    for (int n = 0; n < N; ++n) {
    buf[n]           =  in[n];        // x_e[n]      =  x[n]
    buf[2 * N - 1 - n] =  in[n];      // x_e[2N-1-n] =  x[n]
    }

    /* 2. 對長度 2N 的序列做 FFT ---------------------------------------- */
    fft(buf, /*inverse=*/false);

    /* 3. 從 FFT 結果取實部並補偏移，得到 DCT-II ------------------------ */
    out.resize(N);
    for (int k = 0; k < N; ++k) {
    cd twiddle = std::exp(cd(0, -PI * k / (2.0 * N)));  // e^{-jπk/2N}
    out[k] = (buf[k] * twiddle).real();                 // 只有實數部分
    }

    /* 若想要「正交歸一化」:
    out[0]   *= 1.0 / std::sqrt(2.0);
    for (auto& v : out) v *= std::sqrt(2.0 / N);
    */
}

void BinGrid::idct1d(const std::vector<cd>& in,
    std::vector<cd>&       out)
{
    const int N = static_cast<int>(in.size());
    std::vector<cd> buf(2 * N);

    /* 1. 還原複數頻域向量（與前向步驟完全對偶） ----------------------- */
    for (int k = 0; k < N; ++k) {
    buf[k] = in[k] * std::exp(cd(0,  PI * k / (2.0 * N))); // e^{jπk/2N}
    }
    /* 產生共軛對稱 (Hermitian even) 以確保 IFFT 結果為實數 ----------- */
    for (int k = 1; k < N; ++k)                // k=0 不重複
    buf[2 * N - k] = std::conj(buf[k]);

    /* 2. IFFT ----------------------------------------------------------- */
    fft(buf, /*inverse=*/true);                // 傳入 same buf, in-place

    /* 3. 取前 N 點實部即為重建訊號 ------------------------------ */
    out.resize(N);
    for (int n = 0; n < N; ++n)
    out[n] = buf[n].real() / (2.0 * N);    // /2N 抵銷 forward 未縮放

    /* 若你在 forward 端有做「正交歸一化」，這裡要把同樣的係數反向乘回去 */
}

void BinGrid::dst1d(const std::vector<cd>& in, std::vector<cd>& out) {
    const int N = in.size();
    vector<cd> buf(2*N + 2);           // 長度 2N+2

    buf[0] = buf[N+1] = cd(0,0);            // 端點 0
    for (int n = 0; n < N; ++n) {
        // odd 鏡像：x[1+n] =  f[n] ,  x[2N+1-(1+n)] = -f[n]
        buf[n+1]           = in[n];
        buf[2*N+1-(n+1)]   = -in[n];
    }

    fft(buf, /*invert=*/false);             // FFT

    out.resize(N);
    for (int k = 0; k < N; ++k)
        // 公式：S[k] = (−Im X[k+1]) /2
        out[k] = -0.5 * buf[k+1].imag();
}

void BinGrid::idst1d(const std::vector<cd>& in, std::vector<cd>& out) {
    const int N = in.size();
    vector<cd> buf(2*N + 2, cd(0,0));

    for (int k = 0; k < N; ++k) {
        // 按奇對稱填入虛部
        buf[k+1]         = cd(0,-1) * in[k];       // imag = -S[k]
        buf[2*N+1-(k+1)] = cd(0,1) * in[k];       // conjugate
    }

    fft(buf, /*invert=*/true);                  // IFFT (已 / (2N+2))

    out.resize(N);
    for (int n = 0; n < N; ++n)
        out[n] = buf[n+1].real();               // 取實部
}
