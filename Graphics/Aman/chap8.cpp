#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "camera.h"
#define drand48() ((double) rand()/RAND_MAX)

vec3 random_in_unit_sphere() {
    vec3 p;
    do {
        p=2.0*vec3(drand48(), drand48(), drand48()) - vec3(1,1,1);
    } while (p.squared_length() >= 1.0);
    return p;
}

vec3 color(const ray& r, hitable *world, int depth){
    hit_record rec;
    if(world -> hit(r,0.001, FLT_MAX, rec)) {
        ray scattered;
        vec3 attenuation;
        if(depth < 50 && rec.mat_ptr -> scatter(r,rec,attenuation,scattered)){
            return attenuation*color(scattered, world, depth+1);
        }
        else {
            return vec3(0,0,0);
        }
    }
    else {
        vec3 unit_direction = unit_vector(r.direction());
        float t= 0.5*(unit_direction.y() +1.0);
        return (1.0 -t)*vec3(1.0,1.0,1.0) + t*vec3(0.5,0.7,1.0);
    }
}

class lambertain: public material {
    public:
        lambertain (const vec3& a) : albedo(a) {}
        virtual bool scatter( const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const {
            vec3 target = rec.p + rec.normal + random_in_unit_sphere();
            scattered = ray(rec.p, target-rec.p);
            attenuation = albedo;
            return true;
        }
        vec3 albedo;
};

vec3 reflect(const vec3& v, const vec3& n) {
    return v-2*dot(v,n)*n;
}

class metal : public material {
    public:
        metal(const vec3& a) : albedo(a) {}
        virtual bool scatter(const ray& r_in,const hit_record& rec, vec3& attenuation, ray& scattered) const {
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected);
            attenuation = albedo;
            return (dot(scattered.direction(), rec.normal)>0);
        }
        vec3 albedo;
        float fuzz;
};

int main(){
    std::ofstream fout("chap8.ppm");
    if(fout.fail()) return -1;

    int nx=200;
    int ny=100;
    int ns=100;
    fout<<"P3\n"<<nx<<" "<<ny<<"\n255\n";
    static lambertain diffuse_grey(vec3(0.8,0.3,0.3));
    static lambertain diffuse_grey1(vec3(0.8,0.8,0.0));
    static metal reddish_metal(vec3(0.8,0.6,0.2));
    static metal grey_metal(vec3(0.8,0.8,0.8));
    hitable *list[4];
    list[0] = new sphere(vec3(0,0,-1),0.5,&diffuse_grey);
    list[1] = new sphere(vec3(0,-100.5,-1),100, &diffuse_grey1);
    list[2] = new sphere(vec3(1,0,-1),0.5, &reddish_metal);
    list[3] = new sphere(vec3(-1,0,-1), 0.5,&grey_metal);
    hitable *world = new hitable_list(list,4);
    camera cam;
    for(int j=ny-1;j>=0;j--){
        for(int i=0;i<nx;i++){
            vec3 col(0,0,0);
            for(int s=0;s<ns;s++){
                float u = float(i + drand48())/ float(nx);
                float v= float(j+ drand48()) / float(ny);
                ray r = cam.get_ray(u,v);
                vec3 p = r.point_at_parameter(2.0);
                col += color(r,world,0);
            }
            col/= float(ns);
            col = vec3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );
            int ir = int(255.99*col[0]);
            int ig = int(255.99*col[1]);
            int ib = int(255.99*col[2]);

            fout<<ir<<" "<<ig<<" "<<ib<<"\n";
        }
    }
    std::cout<<"Successful\n";
    return 0;
}