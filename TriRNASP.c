#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <omp.h>
#include <ctype.h>
#define TABLE_SIZE 131071   
#define num 1000    
#define path_l 300  

static double *g_energy = NULL;

static int cmp_idx(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    double ea = g_energy[ia];
    double eb = g_energy[ib];
    return (ea > eb) - (ea < eb);   
}

static int   num_dists;     
double Mu[1];
typedef struct Node {
    int    key[6];        
    uint32_t *counts;     
    struct Node *next;
} Node;

static Node *table[TABLE_SIZE] = { NULL };


static unsigned hash6(const int k[6]) {
    uint64_t h = 1465271;
    for(int i=0;i<6;++i) h = h*31 + (uint32_t)k[i];
    return (unsigned)(h % TABLE_SIZE);
}


void init_table(int distributions) {
    num_dists = distributions;
    memset(table, 0, sizeof(table));
}


static Node *alloc_node() {
    Node *p = malloc(sizeof(Node));
    if (!p) { perror("malloc node"); exit(1); }
    p->counts = calloc(num_dists, sizeof(uint32_t));
    if (!p->counts) { perror("malloc counts"); exit(1); }
    return p;
}


void add_id(const int key[6], int dist_idx) {
    unsigned h = hash6(key);
    for(Node *p = table[h]; p; p = p->next) {
        if (memcmp(p->key, key, 6*sizeof(int))==0) {
            p->counts[dist_idx]++;
            return;
        }
    }

    Node *p = alloc_node();
    memcpy(p->key, key, 6*sizeof(int));
    p->counts[dist_idx] = 1;
    p->next = table[h];
    table[h] = p;
}


static inline double key_to_x(const int key[6]) {

    return (double)(sqrt(1.0*key[5]*key[5] +1.0*key[4]*key[4] + 1.0*key[3]*key[3]) + 1.0*key[2] + 1.0*key[1] + 1.0*key[0]);
}


double compute_entropy_gauss(int dist_idx) {

    uint64_t N = 0;
    for (unsigned i = 0; i < TABLE_SIZE; ++i)
        for (Node *p = table[i]; p; p = p->next)
            N += p->counts[dist_idx];
    if (N == 0) return 0.0;


    double mu = 0.0;
    for (unsigned i = 0; i < TABLE_SIZE; ++i) {
        for (Node *p = table[i]; p; p = p->next) {
            uint32_t c = p->counts[dist_idx];
            if (c == 0) continue;
            double pi = (double)c / (double)N;
            double xi = key_to_x(p->key);
            mu += pi * xi;
        }
    }


    double var = 0.0;
    for (unsigned i = 0; i < TABLE_SIZE; ++i) {
        for (Node *p = table[i]; p; p = p->next) {
            uint32_t c = p->counts[dist_idx];
            if (c == 0) continue;
            double pi = (double)c / (double)N;
            double xi = key_to_x(p->key);
            double d = xi - mu;
            var += pi * d * d;
        }
    }

    if (var <= 0) var = 1e-8;


    double Z = 0.0;
    for (unsigned i = 0; i < TABLE_SIZE; ++i) {
        for (Node *p = table[i]; p; p = p->next) {
            if (p->counts[dist_idx] == 0) continue;
            double xi = key_to_x(p->key);
            double exponent = - (xi - mu)*(xi - mu) / (2.0 * var);
            Z += exp(exponent);
        }
    }


    double D = 0.0;
    for (unsigned i = 0; i < TABLE_SIZE; ++i) {
        for (Node *p = table[i]; p; p = p->next) {
            uint32_t c = p->counts[dist_idx];
            if (c == 0) continue;
            double pi = (double)c / (double)N;
            double xi = key_to_x(p->key);
            double exponent = - (xi - mu)*(xi - mu) / (2.0 * var);
            double qi = exp(exponent) / Z;
            D += pi * log(pi / qi);
        }
    }
    return D;  
}


void free_table() {
    for(unsigned i=0;i<TABLE_SIZE;i++){
        Node *p = table[i];
        while(p){
            Node *n = p->next;
            free(p->counts);
            free(p);
            p = n;
        }
        table[i] = NULL;
    }
}


static inline int compute_code(const char *s) {
    if (!s || !s[0]) return -1;

    int base_idx;
    switch (s[0]) {
        case 'A': base_idx = 0; break;
        case 'U': base_idx = 1; break;
        case 'C': base_idx = 2; break;
        case 'G': base_idx = 3; break;
        default:  return -1;
    }


    if (s[1] == 'P' && s[2] == '\0') {
        return 8 + base_idx;
    }


    if (s[1] == 'C' && s[2] == '4' && s[3] == '\'' && s[4] == '\0') {
        return base_idx;
    }


    if (s[1] == 'N' && s[3] == '\0') {
        if (s[2] == '9') {

            return (s[0] == 'A' || s[0] == 'G') ? (4 + base_idx) : -1;
        }
        if (s[2] == '1') {

            return (s[0] == 'U' || s[0] == 'C') ? (4 + base_idx) : -1;
        }
    }

    return -1;
}


int read_file(char file_name[num][path_l], char file_name1[num][path_l], char path1[path_l])
{
    DIR *dir;
    struct dirent *ptr;
    int n = 0;
    char path[path_l];


    snprintf(path, sizeof(path), "%s", path1);
    size_t plen = strlen(path);
    if (plen == 0) { fprintf(stderr, "input path\n"); return 0; }
    if (path[plen-1] != '/') {
        if (plen + 1 >= sizeof(path)) {  
            fprintf(stderr, "the path too long: %s\n", path1);
            return 0;
        }
        path[plen] = '/'; path[plen+1] = '\0';
        ++plen;
    }

    printf("Scanning  directory: %s\n", path);
    dir = opendir(path);
    if (!dir) { perror("opendir"); return 0; }

    while ((ptr = readdir(dir)) != NULL) {
        const char *name = ptr->d_name;
        size_t L = strlen(name);
        if (name[0]=='.') continue;
        if (L < 4) continue;
        if (strcmp(name + L - 4, ".pdb") != 0) continue;


        if (plen + L + 1 > path_l) {
            fprintf(stderr, "the path too long，next: %s%s\n", path, name);
            continue;
        }


        memcpy(file_name[n], path, plen);
        memcpy(file_name[n] + plen, name, L + 1); 


        if (L + 1 > path_l) {
            fprintf(stderr, "file name too long，next: %s\n", name);
            continue;
        }
        memcpy(file_name1[n], name, L + 1);

        if (++n >= num) break;
    }
    closedir(dir);
    printf("Found PDBs: %d\n", n);
    return n;
}


int main(int argc, char *argv[])
{

if (argc < 2) {
    fprintf(stderr, "Usage: %s <pdb_folder_directory>\n", argv[0]);
    return 1;
}

    double t0 = omp_get_wtime();


double R0_orig = 8.0;
double bin_width = 2.0;
double bin_width1 = 0.6;


int intervals = (int)floor(R0_orig / bin_width + 0.5);
int intervals1 = (int)floor(R0_orig / bin_width1 + 0.5);
if (intervals < 1) intervals = 1;
if (intervals1 < 1) intervals1 = 1;


    char file_name[num][path_l], file_name1[num][path_l], aline[500];
    FILE *fp, *fpotential_nature, *fpotential_nature1;
    int a, b, c, a1, b1, c1;
    int K, k, L, i, n1, n2, n3, n4, n5, n6;

    

    double potentialall[10000];
   
   
const int D1 = 12, D2 = 12, D3 = 12;
const int I  = intervals;
const int I1  = intervals1;

size_t total = (size_t)D1 * D2 * D3 * I * I * (2*I);
size_t total1 = (size_t)D1 * D2 * D3 * I1 * I1 * (2*I1);


double *potential_nature = malloc(total * sizeof(double));
double *potential_nature1 = malloc(total1 * sizeof(double));
if (!potential_nature) {
    perror("malloc potential_nature");
    exit(EXIT_FAILURE);
}
if (!potential_nature1) {
    perror("malloc potential_nature1");
    exit(EXIT_FAILURE);
}

#define IDX(n1,n2,n3,n4,n5,n6) \
    ((((((size_t)(n1)*D2 + (n2))*D3 + (n3))*I + (n4))*I + (n5))*(2*I) + (n6))
#define IDX1(n1,n2,n3,n4,n5,n6) \
    ((((((size_t)(n1)*D2 + (n2))*D3 + (n3))*I1 + (n4))*I1 + (n5))*(2*I1) + (n6))

    

    a = 0;
    b = 0;
    c = 0;
    

    fpotential_nature = fopen("Energy/Rough.energy", "r");
    if (!fpotential_nature) { perror("open Rough.energy"); return 1; }
    while (!feof(fpotential_nature))
    {
        for (n1 = 0; n1 < 12; n1++)
            for (n2 = 0; n2 < 12; n2++)
                for (n3 = 0; n3 < 12; n3++)
                

                for (n4 = 0; n4 < intervals; n4++)
                { 
                for (n5 = 0; n5 < intervals; n5++)
                { 
                if (abs(n4 - n5) == 0){
                for (n6 = 0; n6 <= n4 + n5 + 1; n6++)
                { 
                    int tmp2 = fscanf(fpotential_nature, "%d %d %d %d %d %d %lf", &a, &b, &c, &a1, &b1, &c1, &potential_nature[ IDX(n1,n2,n3,n4,n5,n6) ]);
                    (void)tmp2;
                }
                }
                else{
                for (n6 = abs(n4 - n5)-1; n6 <= n4 + n5 + 1; n6++)
                { 
                    int tmp2 = fscanf(fpotential_nature, "%d %d %d %d %d %d %lf", &a, &b, &c, &a1, &b1, &c1, &potential_nature[ IDX(n1,n2,n3,n4,n5,n6) ]);
                    (void)tmp2;
                }
                }
                }
                }
    }
    fclose(fpotential_nature); 


    fpotential_nature1 = fopen("Energy/Fine.energy", "r");
    if (!fpotential_nature1) { perror("open Fine.energy"); return 1; }
    while (!feof(fpotential_nature1))
    {
        for (n1 = 0; n1 < 12; n1++)
            for (n2 = 0; n2 < 12; n2++)
                for (n3 = 0; n3 < 12; n3++)
                

                for (n4 = 0; n4 < intervals1; n4++)
                { 
                for (n5 = 0; n5 < intervals1; n5++)
                { 
                if (abs(n4 - n5) == 0){
                for (n6 = 0; n6 <= n4 + n5 + 1; n6++)
                { 
                    int tmp2 = fscanf(fpotential_nature1, "%d %d %d %d %d %d %lf", &a, &b, &c, &a1, &b1, &c1, &potential_nature1[ IDX1(n1,n2,n3,n4,n5,n6) ]);
                    (void)tmp2;
                }
                }
                else{
                for (n6 = abs(n4 - n5)-1; n6 <= n4 + n5 + 1; n6++)
                { 
                    int tmp2 = fscanf(fpotential_nature1, "%d %d %d %d %d %d %lf", &a, &b, &c, &a1, &b1, &c1, &potential_nature1[ IDX1(n1,n2,n3,n4,n5,n6) ]);
                    (void)tmp2;
                }
                }
                }
                }
    }
    fclose(fpotential_nature1); 
    
    memset(file_name, 0, sizeof(file_name));
    memset(file_name1, 0, sizeof(file_name1));
    memset(potentialall, 0, sizeof(potentialall));
    Mu[0] = 0.0;


    K = read_file(file_name, file_name1, argv[1]);

    const int M = K;        
    init_table(M);    
  
    for (k = 0; k < K; k++)
    {   

        L = 0;
        fp = fopen(file_name[k], "r+");
        while (fgets(aline, 500, fp) != NULL)
        {
            L++;
            memset(aline, 0, sizeof(aline));
        }
        fclose(fp);

        char type1[L][10], type2[L][10], type[L][10], chain[L][20], num2[L][20];
        double x[L], y[L], z[L];
        char x1[L][10], y1[L][10], z1[L][10];
        memset(type1, 0, sizeof(type1));
        memset(type2, 0, sizeof(type2));
        memset(type, 0, sizeof(type));
        memset(chain, 0, sizeof(chain));
        memset(num2, 0, sizeof(num2));
        memset(x, 0, sizeof(x));
        memset(y, 0, sizeof(y));
        memset(z, 0, sizeof(z));
        memset(x1, 0, sizeof(x1));
        memset(y1, 0, sizeof(y1));
        memset(z1, 0, sizeof(z1));
    
        memset(aline, 0, sizeof(aline));
        fp = fopen(file_name[k], "r");
        if (!fp) { perror(file_name[k]); continue; }
        i = 0;
        while (fgets(aline, 500, fp) != NULL)
        {

            if (aline[0] == 'A' && aline[1] == 'T' && aline[2] == 'O' && aline[3] == 'M')
            {

                sprintf(type1[i], "%c%c%c", aline[13], aline[14], aline[15]);


                {
                    char tmp[10];
                    strcpy(tmp, type1[i]);
                    int spaceCount = 0, e;
                    for (e = 0; e < (int)strlen(tmp); e++) {
                        if (tmp[e] == ' ')
                            spaceCount++;
                        else
                            break;
                    }
                    if (spaceCount > 0) {
                        for (e = 0; e < (int)strlen(tmp) - spaceCount + 1; e++) {
                            type1[i][e] = tmp[e + spaceCount];
                        }
                    }
                }


                sprintf(type2[i], "%c", aline[19]);

                sprintf(type[i], "%s%s", type2[i], type1[i]);
                
                sprintf(chain[i], "%c", aline[21]);

                sprintf(num2[i], "%c%c%c%c", aline[22], aline[23], aline[24], aline[25]);

                sprintf(x1[i], "%c%c%c%c%c%c%c%c", aline[30], aline[31], aline[32], aline[33], aline[34], aline[35], aline[36], aline[37]);
                sprintf(y1[i], "%c%c%c%c%c%c%c%c", aline[38], aline[39], aline[40], aline[41], aline[42], aline[43], aline[44], aline[45]);
                sprintf(z1[i], "%c%c%c%c%c%c%c%c", aline[46], aline[47], aline[48], aline[49], aline[50], aline[51], aline[52], aline[53]);
                x[i] = atof(x1[i]);
                y[i] = atof(y1[i]);
                z[i] = atof(z1[i]);


char *src = type[i], *dst = type[i];
while (*src) {
    if (*src != ' ') {
        *dst++ = *src;
    }
    src++;
}
*dst = '\0';  
        
                
                i++;
            }
            memset(aline, 0, sizeof(aline));
        }
        fclose(fp);

    
int num2_int[i];
for (int j = 0; j < i; ++j) {
    num2_int[j] = atoi(num2[j]);  
}


        int type_code[i];
        for (n1 = 0; n1 < i; n1++) {
            type_code[n1] = compute_code(type[n1]);
            if(type_code[n1] >= 0 && type_code[n1] <= 3 ) Mu[0] += 1.0; //n_BASE
        }
    



double R0_sq_orig = (R0_orig-0.3)*(R0_orig-0.3);
double inv_bw = 1.0 / bin_width;
double sum    = 0.0;
double inv_bw1 = 1.0 / bin_width1;

#pragma omp parallel for reduction(+:sum) schedule(dynamic,1)
for (int n1 = 0; n1 < i; ++n1) {
    int key[6];
    for (int n2 = n1 + 1; n2 < i; ++n2) {
    
            double d12 = (x[n1]-x[n2])*(x[n1]-x[n2])
                       + (y[n1]-y[n2])*(y[n1]-y[n2])
                       + (z[n1]-z[n2])*(z[n1]-z[n2]);
           
        if (d12 >= R0_sq_orig) continue;
                
        for (int n3 = n2 + 1; n3 < i; ++n3) {


            double d13 = (x[n1]-x[n3])*(x[n1]-x[n3])
                       + (y[n1]-y[n3])*(y[n1]-y[n3])
                       + (z[n1]-z[n3])*(z[n1]-z[n3]);

        if (d13 >= R0_sq_orig) continue;
                
            double d23 = (x[n2]-x[n3])*(x[n2]-x[n3])
                       + (y[n2]-y[n3])*(y[n2]-y[n3])
                       + (z[n2]-z[n3])*(z[n2]-z[n3]);



           if(d12 <= 1.21 || d13 <= 1.21 || d23 <= 1.21) {sum += 0.5; continue;}  
           if( ( d12 <= 2.89 && d12 > 1.21) && abs(num2_int[n1] - num2_int[n2]) > 1) {sum += 0.5; continue;}
           if( ( d13 <= 2.89 && d13 > 1.21) && abs(num2_int[n1] - num2_int[n3]) > 1) {sum += 0.5; continue;}
           if( ( d23 <= 2.89 && d23 > 1.21) && abs(num2_int[n3] - num2_int[n2]) > 1) {sum += 0.5; continue;}                    
           if( (d12 > 2.89 && d12 <= 4.0) || (d13 > 2.89 && d13 <= 4.0) || (d23 > 2.89 && d23 <= 4.0)) {sum += 0.2; continue;}
           
           
            int b12 = (int)(sqrt(d12) * inv_bw);
            int b23 = (int)(sqrt(d23) * inv_bw);
            int b13 = (int)(sqrt(d13) * inv_bw);

            int D12 = (int)(sqrt(d12) * inv_bw1);
            int D23 = (int)(sqrt(d23) * inv_bw1);
            int D13 = (int)(sqrt(d13) * inv_bw1);

if (b12 > intervals - 1 || b13 > intervals - 1 || b23 > 2*intervals - 1) continue;


            if(type_code[n1] >= 0 && type_code[n2] >= 0 && type_code[n3] >= 0)
            {
                        sum += potential_nature[
                        IDX(type_code[n1], type_code[n2], type_code[n3], b12, b13, b23)];

                        key[0] = type_code[n1];
                        key[1] = type_code[n2];
                        key[2] = type_code[n3];
                        key[3] = D12;
                        key[4] = D13;
                        key[5] = D23;
                        #pragma omp critical
                        add_id(key, k);
            } else continue;
        }
    }
}


potentialall[k] = sum;

    }
    

Mu[0] = Mu[0]/K;

for(int d=0; d<K; d++){

        potentialall[d] += 1.0*Mu[0]*compute_entropy_gauss(d);
    }
    


        int *idx = malloc(K * sizeof(int));
        if (!idx) {
            perror("malloc idx");
            return EXIT_FAILURE;
        }
        for (int i = 0; i < K; i++) idx[i] = i;

g_energy = potentialall;
qsort(idx, K, sizeof *idx, cmp_idx);


int keep = 5;
if (keep < 1) keep = 1;
if (keep > K) keep = K;

if (K <= 0) {
    fprintf(stderr, "Error: K = %d Make Nonsence，Can not create 'included'.\n", K);
    free(idx);
    return EXIT_FAILURE;
}

char *included = calloc((size_t)K, sizeof(char));
if (!included) {
    perror("calloc included");
    free(idx);
    return EXIT_FAILURE;
}

int count_written = 0;
for (int it = 0; it < K && count_written < keep; it++) {  
    int id = idx[it];
    if (included[id]) continue; 
    included[id] = 1;
    count_written++;


    int L = 0;
    FILE *fp = fopen(file_name[id], "r");   
    if (!fp) { perror(file_name[id]); continue; }
    char aline[500];
    while (fgets(aline, sizeof(aline), fp)) L++;
    fclose(fp);


    char   type1[L][10], type2[L][10], type[L][10], chain[L][20], num2[L][20];
    double x[L], y[L], z[L];
    char   x1[L][10], y1[L][10], z1[L][10];
    memset(type1,0,sizeof(type1)); memset(type2,0,sizeof(type2));
    memset(type,0,sizeof(type));   memset(chain,0,sizeof(chain));
    memset(num2,0,sizeof(num2));   memset(x,0,sizeof(x));
    memset(y,0,sizeof(y));         memset(z,0,sizeof(z));
    memset(x1,0,sizeof(x1));       memset(y1,0,sizeof(y1)); memset(z1,0,sizeof(z1));


    fp = fopen(file_name[id], "r");
    if (!fp) { perror(file_name[id]); continue; }
    int ia = 0;                                  
    while (fgets(aline, sizeof(aline), fp)) {
        if (!(aline[0]=='A' && aline[1]=='T' && aline[2]=='O' && aline[3]=='M')) continue;
        if ((int)strlen(aline) < 54) continue;

        sprintf(type1[ia], "%c%c%c", aline[13], aline[14], aline[15]);
        {  
            char tmp[10]; strcpy(tmp, type1[ia]);
            int e=0; while (tmp[e]==' ') e++;
            strcpy(type1[ia], tmp+e);
        }
        sprintf(type2[ia], "%c", aline[19]);
        sprintf(type[ia],  "%s%s", type2[ia], type1[ia]);

        sprintf(chain[ia], "%c", aline[21]);
        sprintf(num2[ia],  "%c%c%c%c", aline[22], aline[23], aline[24], aline[25]);

        sprintf(x1[ia], "%c%c%c%c%c%c%c%c", aline[30], aline[31], aline[32], aline[33], aline[34], aline[35], aline[36], aline[37]);
        sprintf(y1[ia], "%c%c%c%c%c%c%c%c", aline[38], aline[39], aline[40], aline[41], aline[42], aline[43], aline[44], aline[45]);
        sprintf(z1[ia], "%c%c%c%c%c%c%c%c", aline[46], aline[47], aline[48], aline[49], aline[50], aline[51], aline[52], aline[53]);

        x[ia] = atof(x1[ia]); y[ia] = atof(y1[ia]); z[ia] = atof(z1[ia]);


        for (char *src=type[ia], *dst=type[ia]; ; src++) { if (*src!=' ') *dst++=*src; if(!*src){*dst='\0';break;} }

        ia++;
    }
    fclose(fp);


    int num2_int[ia];
    for (int j = 0; j < ia; ++j) num2_int[j] = atoi(num2[j]);


    int type_code[ia];
    for (int j = 0; j < ia; ++j) type_code[j] = compute_code(type[j]);


    double R0_sq_orig = (R0_orig-0.3)*(R0_orig-0.3);
    double inv_bw = 1.0 / bin_width1;
    double sum = 0.0;

    #pragma omp parallel for reduction(+:sum) schedule(dynamic,1)
    for (int n1 = 0; n1 < ia; ++n1) {
        for (int n2 = n1 + 1; n2 < ia; ++n2) {
            double dx = x[n1]-x[n2], dy = y[n1]-y[n2], dz = z[n1]-z[n2];
            double d12 = dx*dx + dy*dy + dz*dz;
            if (d12 >= R0_sq_orig) continue;

            for (int n3 = n2 + 1; n3 < ia; ++n3) {
                double d13 = (x[n1]-x[n3])*(x[n1]-x[n3]) + (y[n1]-y[n3])*(y[n1]-y[n3]) + (z[n1]-z[n3])*(z[n1]-z[n3]);
                if (d13 >= R0_sq_orig) continue;
                double d23 = (x[n2]-x[n3])*(x[n2]-x[n3]) + (y[n2]-y[n3])*(y[n2]-y[n3]) + (z[n2]-z[n3])*(z[n2]-z[n3]);

                if(d12 <= 1.21 || d13 <= 1.21 || d23 <= 1.21) {sum += 0.5; continue;}
                if((d12 <= 2.89 && d12 > 1.21) && abs(num2_int[n1]-num2_int[n2]) > 1) {sum += 0.5; continue;}
                if((d13 <= 2.89 && d13 > 1.21) && abs(num2_int[n1]-num2_int[n3]) > 1) {sum += 0.5; continue;}
                if((d23 <= 2.89 && d23 > 1.21) && abs(num2_int[n3]-num2_int[n2]) > 1) {sum += 0.5; continue;}
                if((d12 > 2.89 && d12 <= 4.0) || (d13 > 2.89 && d13 <= 4.0) || (d23 > 2.89 && d23 <= 4.0)) {sum += 0.2; continue;}

                int b12 = (int)(sqrt(d12) * inv_bw);
                int b13 = (int)(sqrt(d13) * inv_bw);
                int b23 = (int)(sqrt(d23) * inv_bw);
                if (b12 > intervals1 - 1 || b13 > intervals1 - 1 || b23 > 2*intervals1 - 1) continue;

            if(type_code[n1] >= 0 && type_code[n2] >= 0 && type_code[n3] >= 0)
            {
                        sum += potential_nature1[ 
                        IDX1(type_code[n1],type_code[n2],type_code[n3],b12,b13,b23) ];

            } else continue;
            }
        }
    }


    printf("%s   %.12lf\n", file_name1[id], sum);
}

free(idx);
free(included);

    

    
    free(potential_nature);
    free(potential_nature1);
    double t1 = omp_get_wtime();
    printf("Wall-clock time: %f seconds\n", t1 - t0);

    free_table();
    

    return 0;
}
