#include "evaluation.h"

evaluation::evaluation(data *gdata)
{
    ned = gdata;
    N = ned->n_vertices;
}

void evaluation::load_data(char *visfile)
{
    data d;
    vis =  d.load_vec(visfile, n_vertices, out_dim);
}

bool operator < (const Dis &d1, const Dis &d2){
    return d1.distance < d2.distance;
};

vector<int> evaluation::vectors_intersection(vector<int> v1,vector<int> v2)
{
    vector<int> v;
    sort(v1.begin(),v1.end());
    sort(v2.begin(),v2.end());
    set_intersection(v1.begin(),v1.end(),v2.begin(),v2.end(),back_inserter(v));
    return v;
}

vector<int> evaluation::vectors_union(vector<int> v1,vector<int> v2)
{
    vector<int> v;
    sort(v1.begin(),v1.end());
    sort(v2.begin(),v2.end());
    set_union(v1.begin(),v1.end(),v2.begin(),v2.end(),back_inserter(v));
    return v;
}

real evaluation::CalcDist2D(int x, int y)
{
    real ret = 0;
    int i, lx = x * out_dim, ly = y * out_dim;
    for (i = 0; i < out_dim; i++)
    {
        ret += (vis[lx + i] - vis[ly + i]) * (vis[lx + i] - vis[ly + i]);
    }
    return ret;
}


void *evaluation::accuracy_thread_caller(void* arg)
{
    evaluation* ptr = (evaluation*)(((arg_evaluation*)arg)->ptr);
    ptr->accuracy_thread(((arg_evaluation*)arg)->id);
    pthread_exit(NULL);
}

void evaluation::accuracy_thread(int id)
{
    for (int i = id; i < N; i = i + n_threads)
    {
        int k_neighbors = 0;
        float temp = 0, dis_t = 0;

        std::vector<Dis> distance;

        vector<int> Ng;
        vector<int> Ny;

        vector<int> v_inter,v_uion;

        //Ng Graph
        int p, q;
        for (p = ned->head[i]; p >= 0; p = ned->next[p])
        {
            q = ned->edge_to[p];
            Ng.push_back(q);
        }

        k_neighbors = (int)Ng.size();

        for(int j = 0; j < N; j++)
        {
            if(i != j)
            {
                temp = CalcDist2D(i, j);
                if(distance.size() < k_neighbors)
                {
                    distance.push_back(Dis(j,temp));
                    if(temp > dis_t)
                    {
                        dis_t = temp;
                    }
                }
                else
                {
                    if(temp >= dis_t)
                    {
                        continue;
                    }
                    else
                    {
                        //dis_t = temp;
                        distance.push_back(Dis(j,temp));
                        std::sort(distance.begin(), distance.end());
                        dis_t = distance[k_neighbors].distance;
                        distance.erase(distance.end());
                    }
                }
            }
            else
                continue;
        }

        for(int t = 0; t < k_neighbors; ++t)
        {
            Ny.push_back(distance[t].index);
        }

        v_uion = vectors_union(Ng,Ny);
        v_inter = vectors_intersection(Ng,Ny);

        sum[i] = v_inter.size() * 1.0 / v_uion.size();
    }
}

float evaluation::jaccard()
{
    float sum_all = 0;
    sum = new float[N];
    pthread_t *pt = new pthread_t[n_threads];
    for (int j = 0; j < n_threads; ++j){
        pthread_create(&pt[j], NULL, evaluation::accuracy_thread_caller, new arg_evaluation(this, j));
    }
    for (int j = 0; j < n_threads; ++j){
        pthread_join(pt[j], NULL);
    }
    delete[] pt;

    int count = 0;
    for (int i = 0; i < N; ++i)
    {
        if (sum[i] != -1){
            sum_all += sum[i];
            count++;
        }
    }

    average = sum_all / count;

    cout << average << endl;

    delete []sum;
    return average;
}


