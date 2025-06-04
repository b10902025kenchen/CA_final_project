#ifndef BINGRID_H
#define BINGRID_H
#include "Bin.h"
#include <vector>
#include <complex>
#include "Point.h"
#include "Star.h"

using namespace std;
#define cd std::complex<double>
#define PI 3.14159265358979323846

class BinGrid
{
    public:
    vector<vector<Bin>> _Bin2D;
    vector<Star*> _pModules;
    vector<Point2<double>> _field_cache;
    int _num_bins_x;
    int _num_bins_y;
    double len_x;
    double len_y; 
    double lower_x;
    double lower_y;
    float ovfl;
    float target_density;
    double maximal_weight = 0.0; // maximal weight of a star

    // interface with FFT LIBRARY
    float** density_map;
    float** electroPhi_;
    float** field_x;
    float** field_y;
    vector<float> _cos_table;
    vector<int> ip_table;

    BinGrid(double x0, double x1, double y0, double y1, vector<Star*> stars);
    ~BinGrid();
    void rescale(int num_bin_x, int num_bin_y);
    int get_num_bins_x() {return _num_bins_x;}
    int get_num_bins_y() {return _num_bins_y;}
    BinGrid(){}

    pair<int,int> getBinIdx(double x, double y)
    {
        int idx_x = (x - _Bin2D[0][0].l_x) / _Bin2D[0][0].len_x();
        int idx_y = (y - _Bin2D[0][0].l_y) / _Bin2D[0][0].len_y();
        return make_pair(idx_x, idx_y);
    }
    pair<pair<int,int>, pair<int,int>> getBinIdx(Star* module)
    {
        int l_idx_x = (module->x() - _Bin2D[0][0].l_x) / _Bin2D[0][0].len_x();
        int l_idx_y = (module->y() - _Bin2D[0][0].l_y) / _Bin2D[0][0].len_y();
        int u_idx_x = (module->x() + module->width() - _Bin2D[0][0].l_x) / _Bin2D[0][0].len_x();
        int u_idx_y = (module->y() + module->height() - _Bin2D[0][0].l_y) / _Bin2D[0][0].len_y();
        if(l_idx_x < 0) l_idx_x = 0;
        if(l_idx_y < 0) l_idx_y = 0;
        if(u_idx_x >= _num_bins_x) u_idx_x = _num_bins_x - 1;
        if(u_idx_y >= _num_bins_y) u_idx_y = _num_bins_y - 1;
        return make_pair(make_pair(l_idx_x, l_idx_y), make_pair(u_idx_x, u_idx_y));
    }

    void fft(std::vector<cd>& a, bool );
    void apply2DFFT(vector<vector<cd>>&, bool); //2D DCT
    void apply2DDSCT(vector<vector<cd>>&, bool); //2D DSCT S:x C:y
    void apply2DDCST(vector<vector<cd>>&, bool); //2D DCST C:x S:y

    void dct1d(const std::vector<cd>& in, std::vector<cd>& out);
    void idct1d(const std::vector<cd>& in, std::vector<cd>& out);
    void dst1d(const std::vector<cd>& in, std::vector<cd>& out);
    void idst1d(const std::vector<cd>& in, std::vector<cd>& out);

    void initBin2D();
    void updateBin2D();
    void updateBinField();
    void updateBinPhi();
    void normalizeBinField();
    void update_ovfl();
    double get_energy();
    vector<Point2<double>> getBinField();
    vector<Point2<double>> getBinField_cache(){return _field_cache;}

};

/// 1D FFT ////////////////////////////////////////////////////////////////
void cdft(int n, int isgn, float *a, int *ip, float *w);
void ddct(int n, int isgn, float *a, int *ip, float *w);
void ddst(int n, int isgn, float *a, int *ip, float *w);

/// 2D FFT ////////////////////////////////////////////////////////////////
void cdft2d(int, int, int, float **, float *, int *, float *);
void rdft2d(int, int, int, float **, float *, int *, float *);
void ddct2d(int, int, int, float **, float *, int *, float *);
void ddst2d(int, int, int, float **, float *, int *, float *);
void ddsct2d(int n1, int n2, int isgn, float **a, float *t, int *ip, float *w);
void ddcst2d(int n1, int n2, int isgn, float **a, float *t, int *ip, float *w);


#endif // BINGRID_H