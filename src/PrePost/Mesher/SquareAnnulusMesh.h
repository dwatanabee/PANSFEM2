//*****************************************************************************
//  Title       :   src/PrePost/Mesher/SquareAnnulusMesh.h
//  Author      :   Tanabe Yuta
//  Date        :   2020/04/10
//  Copyright   :   (C)2020 TanabeYuta
//*****************************************************************************


#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <algorithm>
#include <cassert>


#include "../../LinearAlgebra/Models/Vector.h"


namespace PANSFEM2{
    //********************SquareAnnulusMesh*******************
    template<class T>
    class SquareAnnulusMesh{
public:
        SquareAnnulusMesh(T _a, T _b, T _c, T _d, int _nx, int _ny, int _nt);
        ~SquareAnnulusMesh();


        std::vector<Vector<T> > GenerateNodes();
        std::vector<std::vector<int> > GenerateElements();
        std::vector<std::vector<int> > GenerateEdges();
        template<class F>
        std::vector<std::pair<std::pair<int, int>, T> > GenerateFixedlist(std::vector<int> _ulist, F _iscorrespond);
private:
        T a, b, c, d;
        int nx, ny, nt, nxy;  
    };


    template<class T>
    SquareAnnulusMesh<T>::SquareAnnulusMesh(T _a, T _b, T _c, T _d, int _nx, int _ny, int _nt){
        this->a = _a;
        this->b = _b;
        this->c = _c;
        this->d = _d;
        this->nx = _nx;
        this->ny = _ny;
        this->nt = _nt;
        this->nxy = 2*(this->nx + this->ny);
    }


    template<class T>
    SquareAnnulusMesh<T>::~SquareAnnulusMesh(){}


    template<class T>
    std::vector<Vector<T> > SquareAnnulusMesh<T>::GenerateNodes(){
        std::vector<Vector<T> > nodes = std::vector<Vector<T> >(this->nxy*(this->nt + 1));
        for(int i = 0; i < this->nt + 1; i++){
            T t = i/(T)this->nt;
            for(int j = 0; j < this->ny; j++){
                nodes[this->nxy*i + j] = { t*0.5*this->a + (1 - t)*0.5*this->c, t*this->b*(j/(T)this->ny - 0.5) + (1 - t)*this->d*(j/(T)this->ny - 0.5) };
                nodes[this->nxy*i + j + this->ny + this->nx] = { -t*0.5*this->a - (1 - t)*0.5*this->c, t*this->b*(0.5 - j/(T)this->ny) + (1 - t)*this->d*(0.5 - j/(T)this->ny) };
            }
            for(int j = 0; j < this->nx; j++){
                nodes[this->nxy*i + j + this->ny] = { t*this->a*(0.5 - j/(T)this->nx) + (1 - t)*this->c*(0.5 - j/(T)this->nx), t*0.5*this->b + (1 - t)*0.5*this->d };
                nodes[this->nxy*i + j + 2*this->ny + this->nx] = { t*this->a*(j/(T)this->nx - 0.5) + (1 - t)*this->c*(j/(T)this->nx - 0.5), -t*0.5*this->b - (1 - t)*0.5*this->d };
            }
        }
        return nodes;
    }


    template<class T>
    std::vector<std::vector<int> > SquareAnnulusMesh<T>::GenerateElements(){
        std::vector<std::vector<int> > elements = std::vector<std::vector<int> >(this->nxy*this->nt);
        for(int i = 0; i < this->nt; i++){
            for(int j = 0; j < this->nxy; j++){
                elements[2*(this->nx + this->ny)*i + j] = { this->nxy*i + j%this->nxy, this->nxy*(i + 1) + j%this->nxy, this->nxy*(i + 1) + (j + 1)%this->nxy, this->nxy*i + (j + 1)%this->nxy };
            }
        }
        return elements;
    }


    template<class T>
    std::vector<std::vector<int> > SquareAnnulusMesh<T>::GenerateEdges(){
        std::vector<std::vector<int> > edges = std::vector<std::vector<int> >(2*this->nxy);
        for(int i = 0; i < this->nxy; i++){
            edges[this->nxy - i - 1] = { (i + 1)%this->nxy, i };
            edges[i + this->nxy] = { this->nxy*this->nt + i, this->nxy*this->nt + (i + 1)%this->nxy };
        }
        return edges;
    }


    template<class T>
    template<class F>
    std::vector<std::pair<std::pair<int, int>, T> > SquareAnnulusMesh<T>::GenerateFixedlist(std::vector<int> _ulist, F _iscorrespond) {
        assert(0 <= *std::min_element(_ulist.begin(), _ulist.end()));
        std::vector<std::pair<std::pair<int, int>, T> > ufixed;
        for(int i = 0; i < this->nt + 1; i++){
            T t = i/(T)this->nt;
            for(int j = 0; j < this->ny; j++){
                if(_iscorrespond(Vector<T>({ (1 - t)*0.5*this->a + t*0.5*this->c, (1 - t)*this->b*(j/(T)this->ny - 0.5) + t*this->d*(j/(T)this->ny - 0.5) }))) {
                    for(auto ui : _ulist) {
                        ufixed.push_back({ { this->nxy*i + j, ui }, T() });
                    }
                }
            }
            for(int j = 0; j < this->nx; j++){
                if(_iscorrespond(Vector<T>({ (1 - t)*this->a*(0.5 - j/(T)this->nx) + t*this->c*(0.5 - j/(T)this->nx), (1 - t)*0.5*this->b + t*0.5*this->d }))) {
                    for(auto ui : _ulist) {
                        ufixed.push_back({ { this->nxy*i + j + this->ny, ui }, T() });
                    }
                }
            }
            for(int j = 0; j < this->ny; j++){
                if(_iscorrespond(Vector<T>({ -(1 - t)*0.5*this->a - t*0.5*this->c, (1 - t)*this->b*(0.5 - j/(T)this->ny) + t*this->d*(0.5 - j/(T)this->ny) }))) {
                    for(auto ui : _ulist) {
                        ufixed.push_back({ { this->nxy*i + j + this->ny + this->nx, ui }, T() });
                    }
                }
            }
            for(int j = 0; j < this->nx; j++){
                if(_iscorrespond(Vector<T>({ (1 - t)*this->a*(j/(T)this->nx - 0.5) + t*this->c*(j/(T)this->nx - 0.5), -(1 - t)*0.5*this->b - t*0.5*this->d }))) {
                    for(auto ui : _ulist) {
                        ufixed.push_back({ { this->nxy*i + j + 2*this->ny + this->nx, ui }, T() });
                    }
                }
            }
        }
        return ufixed;
    }


    //********************SquareAnnulusMesh2*******************
    template<class T>
    class SquareAnnulusMesh2 {
public:
        SquareAnnulusMesh2(T _a, T _b, int _nx, int _ny, int _nv, int _nw);
        ~SquareAnnulusMesh2();


        std::vector<Vector<T> > GenerateNodes();
        std::vector<std::vector<int> > GenerateElements();
        std::vector<std::vector<int> > GenerateEdges();
        template<class F>
        std::vector<std::pair<std::pair<int, int>, T> > GenerateFixedlist(std::vector<int> _ulist, F _iscorrespond);
private:
        T a, b;
        int nx, ny, nv, nw, nxv, nyw; 
    };


    template<class T>
    SquareAnnulusMesh2<T>::SquareAnnulusMesh2(T _a, T _b, int _nx, int _ny, int _nv, int _nw) {
        assert((_nx - _nv)%2 == 0 && (_ny - _nw)%2 == 0);
        this->a = _a;
        this->b = _b;
        this->nx = _nx;
        this->ny = _ny;
        this->nv = _nv;
        this->nw = _nw;

        this->nxv = (this->nx - this->nv)/2;
        this->nyw = (this->ny - this->nw)/2;
    }


    template<class T>
    SquareAnnulusMesh2<T>::~SquareAnnulusMesh2() {}


    template<class T>
    std::vector<Vector<T> > SquareAnnulusMesh2<T>::GenerateNodes() {
        std::vector<Vector<T> > nodes = std::vector<Vector<T> >((this->nx + 1)*(this->ny + 1) - (this->nv - 1)*(this->nw - 1));
        int nodeid = 0;
        for(int i = 0; i < this->nxv + 1; i++) {
            for(int j = 0; j < this->ny + 1; j++) {
                nodes[nodeid] = { this->a*i/(T)this->nx, this->b*j/(T)this->ny };
                nodeid++;
            }
        }
        for(int i = 0; i < this->nv - 1; i++) {
            for(int j = 0; j < this->nyw + 1; j++) {
                nodes[nodeid] = { this->a*(this->nxv + 1 + i)/(T)this->nx, this->b*j/(T)this->ny };
                nodeid++;
            }
            for(int j = 0; j < this->nyw + 1; j++) {
                nodes[nodeid] = { this->a*(this->nxv + 1 + i)/(T)this->nx, this->b*(j + this->nyw + this->nw)/(T)this->ny };
                nodeid++;
            }
        }
        for(int i = 0; i < this->nxv + 1; i++) {
            for(int j = 0; j < this->ny + 1; j++) {
                nodes[nodeid] = { this->a*(this->nxv + this->nv + i)/(T)this->nx, this->b*j/(T)this->ny };
                nodeid++;
            }
        }
        return nodes;
    }


    template<class T>
    std::vector<std::vector<int> > SquareAnnulusMesh2<T>::GenerateElements() {
        std::vector<std::vector<int> > elements = std::vector<std::vector<int> >(this->nx*this->ny - this->nv*this->nw);
        int elementid = 0;
        for(int i = 0; i < this->nxv; i++) {
            for(int j = 0; j < this->ny; j++) {
                elements[elementid] = { (this->ny + 1)*i + j, (this->ny + 1)*(i + 1) + j, (this->ny + 1)*(i + 1) + (j + 1), (this->ny + 1)*i + (j + 1) };
                elementid++;
            }
        }

        for(int j = 0; j < this->nyw; j++) {
            elements[elementid] = { (this->ny + 1)*this->nxv + j, (this->ny + 1)*(this->nxv + 1) + j, (this->ny + 1)*(this->nxv + 1) + (j + 1), (this->ny + 1)*this->nxv + (j + 1) };
            elementid++;
        }

        for(int i = 0; i < this->nv - 1; i++) {
            for(int j = 0; j < this->nyw; j++) {
                elements[elementid] = { (this->ny + 1)*this->nxv + this->nw - 1 + (this->nyw + 1)*(2*i + 1) + j,
                    (this->ny + 1)*this->nxv + this->nw - 1 + (this->nyw + 1)*(2*i + 3) + j,
                    (this->ny + 1)*this->nxv + this->nw - 1 + (this->nyw + 1)*(2*i + 3) + (j + 1),
                    (this->ny + 1)*this->nxv + this->nw - 1 + (this->nyw + 1)*(2*i + 1) + (j + 1)
                };
                elementid++;
            }
            for(int j = 0; j < this->nyw; j++) {
                elements[elementid] = { (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*i + j,
                    (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(i + 1) + j,
                    (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(i + 1) + (j + 1),
                    (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*i + (j + 1),
                };
                elementid++;
            }
        }

        for(int j = 0; j < this->nyw; j++) {
            elements[elementid] = { (this->ny + 1)*(this->nxv + 1) + (this->nyw + 1)*(2*this->nv - 3) + j,
                (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - 1) + this->nyw + this->nw + j, 
                (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - 1) + this->nyw + this->nw + (j + 1),
                (this->ny + 1)*(this->nxv + 1) + (this->nyw + 1)*(2*this->nv - 3) + (j + 1)
            };
            elementid++;
        }

        for(int i = 0; i < this->nxv; i++) {
            for(int j = 0; j < this->ny; j++) {
                elements[elementid] = { (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - 1) + (this->ny + 1)*i + j, 
                    (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - 1) + (this->ny + 1)*(i + 1) + j, 
                    (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - 1) + (this->ny + 1)*(i + 1) + (j + 1), 
                    (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - 1) + (this->ny + 1)*i + (j + 1) };
                elementid++;
            }
        }

        return elements;
    }


    template<class T>
    std::vector<std::vector<int> > SquareAnnulusMesh2<T>::GenerateEdges() {
        std::vector<std::vector<int> > edges = std::vector<std::vector<int> >(2*(this->nx + this->ny));
        int edgeid = 0;
        for(int i = 0; i < this->nxv + 1; i++) {
            edges[edgeid] = { (this->ny + 1)*i, (this->ny + 1)*(i + 1) };
            edgeid++;
        }
        for(int i = 0; i < this->nv - 1; i++) {
            edges[edgeid] = { (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*i, (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(i + 1) };
            edgeid++;
        }
        for(int i = 0; i < this->nxv; i++) {
            edges[edgeid] = { (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - 1) + (this->ny + 1)*i, 
                (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - 1) + (this->ny + 1)*(i + 1) 
            };
            edgeid++;
        }
        for(int j = 0; j < this->ny; j++) {
            edges[edgeid] = { (this->ny + 1)*(2*this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - 1) + j, 
                (this->ny + 1)*(2*this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - 1) + (j + 1) 
            };
            edgeid++;
        }
        for(int i = 0; i < this->nxv + 1; i++) {
            edges[edgeid] = { 2*(this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - 1) - (this->ny + 1)*i - 1, 
                2*(this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - 1) - (this->ny + 1)*(i + 1) - 1 
            };
            edgeid++;
        }
        for(int i = 0; i < this->nv - 1; i++) {
            edges[edgeid] = { (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - i - 1) - 1, 
                (this->ny + 1)*(this->nxv + 1) + 2*(this->nyw + 1)*(this->nv - (i + 1) - 1) - 1
            };
            edgeid++;
        }

        for(int i = 0; i < this->nxv; i++) {
            edges[edgeid] = { (this->ny + 1)*(this->nxv + 1 - i) - 1, (this->ny + 1)*(this->nxv + 1 - (i + 1)) - 1 };
            edgeid++;
        }

        for(int j = 0; j < this->ny; j++) {
            edges[edgeid] = { this->ny - j, this->ny - (j + 1) };
            edgeid++;
        }
        return edges;
    }


    template<class T>
    template<class F>
    std::vector<std::pair<std::pair<int, int>, T> > SquareAnnulusMesh2<T>::GenerateFixedlist(std::vector<int> _ulist, F _iscorrespond) {
        assert(0 <= *std::min_element(_ulist.begin(), _ulist.end()));
        std::vector<std::pair<std::pair<int, int>, T> > ufixed;
        int nodeid = 0;
        for(int i = 0; i < this->nxv + 1; i++) {
            for(int j = 0; j < this->ny + 1; j++) {
                if(_iscorrespond(Vector<T>({ this->a*i/(T)this->nx, this->b*j/(T)this->ny }))) {
                    for(auto ui : _ulist) {
                        ufixed.push_back({ { nodeid, ui }, T() });
                    }
                }
                nodeid++;
            }
        }
        for(int i = 0; i < this->nv - 1; i++) {
            for(int j = 0; j < this->nyw + 1; j++) {
                if(_iscorrespond(Vector<T>({ this->a*(this->nxv + 1 + i)/(T)this->nx, this->b*j/(T)this->ny }))) {
                    for(auto ui : _ulist) {
                        ufixed.push_back({ { nodeid, ui }, T() });
                    }
                }
                nodeid++;
            }
            for(int j = 0; j < this->nyw + 1; j++) {
                if(_iscorrespond(Vector<T>({ this->a*(this->nxv + 1 + i)/(T)this->nx, this->b*(j + this->nyw + this->nw)/(T)this->ny }))) {
                    for(auto ui : _ulist) {
                        ufixed.push_back({ { nodeid, ui }, T() });
                    }
                }
                nodeid++;
            }
        }
        for(int i = 0; i < this->nxv + 1; i++) {
            for(int j = 0; j < this->ny + 1; j++) {
                if(_iscorrespond(Vector<T>({ this->a*(this->nxv + this->nv + i)/(T)this->nx, this->b*j/(T)this->ny }))) {
                    for(auto ui : _ulist) {
                        ufixed.push_back({ { nodeid, ui }, T() });
                    }
                }
                nodeid++;
            }
        }
        return ufixed;
    }
}