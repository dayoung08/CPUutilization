#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <vector>
#include <unordered_set>
#include <set>
//#include <map>
#include <cmath>
#include <ctime>
#include <random> 
using namespace std;

#define DIGIT 100

#define SEED time(NULL)
#define INF 987654321

#define BN_NUM 120// 80 100 default<120> 140 160 180 200
#define BN_TYPE_NUM 10 // 백엔드 노드 타입 추가되면 수정해야 함, 아래도!

#define CHANNEL_NUM 6000 //기본적으로 4000~8000개임, 디폴트 6000
//DP 실험때는 가장 작은 스케일일때를 택함.

//#define VERSION_NUM 7
//#define VERSION_SET_NUM 32 // 오리지널 버전 제외하고, 마지막 버전은 반드시 저장. 2^(7-2) set 1이 오리지널과 마지막 버전만 들어있는 것.
//set 1 = (1)00000(1)이라서 1번, 오리지널 버전만
//set 2 = (1)00001(1)이라서 1번, 오리지널 버전과 함께 2번 버전만,
//set 3 = (1)00010(1)이라서 1번, 오리지널 버전과 함께 3번 버전만.

#define HVP 1
#define MVP 2
#define LVP 3
#define RVP 4

//#define ALPHA 2.0 // 인기도 - 지프 분포에 사용하는 알파 베타 값
//#define BETA 1.0

#define SIGMA 1 // 버전 인기도 - 노멀 분포에 사용하는 값
 //0.25 0.5 0.75 <Default 1> 1.25 1.5 1.75 2 사이의 값.

#define K_gamma 0.399 // 인기도 - 감마 분포에 사용하는 k, 세타값
#define THETA_gamma 14260.0

#define M_E 2.7182818284590452354 /* e */
#define PI 3.1415926535897932384 /* pi */

#define RR_HPF 1
#define RR_AP 2
#define RA_HPF 3
#define RA_AP 4
#define PA_HPF 5
#define PA_AP 6


//210427 for revision
#define VMAF 0
#define SSIM 1
#define PSNR 2
#define MOS 3

class bitrate_version_set_info {
public: //그냥 전부 public 가자
	int index; // 0은 우리가 쓰는 zencoder 조합.

	int version_num;
	int version_set_num;

	int* resolution; //1080p면 값이 1080임
	int* bitrate; // 논문에서의 r을 위한 값. 단 여기선 kbps고, r은 Mbps(GHz 구할 때 변환함). set_GHz 최상단의 논문의 수식 참고.


	float** video_quality;//[CHANNEL_NUM + 1][VERSION_NUM + 1];
	float** popularity;//[CHANNEL_NUM + 1][VERSION_NUM + 1]; // [채널][0] 여기는 전체 채널의 pop, 즉 전체 버전 version의 pop 합.

	float** video_GHz;//[CHANNEL_NUM + 1][VERSION_NUM + 1];
	float** pwq;//[CHANNEL_NUM + 1][VERSION_NUM + 1]; // weighted video quality라고 쓰여있음. 하도 평소에 video pwq이라 불러서 코딩때도 이렇게 함.

	float** video_quality_set;//[CHANNEL_NUM + 1][VERSION_SET_NUM + 1];
	float** pwq_set;//[CHANNEL_NUM + 1][VERSION_SET_NUM + 1];
	//이 set 지우지 말것. pwq 합 계산할때 이걸로 돌리는게 제일 편하다.

	float** sum_of_version_set_GHz;

	bitrate_version_set_info(int _index); //initiation.cpp에 구현 있음
	float get_channel_popularity(int channel);

	int number_for_bit_opration;
	int set_versions_number_for_bit_opration;
};

/* initiation.cpp */
float* set_zipf_pop(int length, float alpha, float beta);
float* set_gamma_pop(float length, float k, float theta);
float* set_version_pop(bitrate_version_set_info* info, int version_pop_type);
void set_metric_score(bitrate_version_set_info* info, int metric_type);
void set_pop(bitrate_version_set_info* info, int version_pop_type);
void set_GHz(bitrate_version_set_info* info);
void segment_init(bitrate_version_set_info* info, int version_pop_type, int metric_type);

/*float get_channel_popularity(int channel);
float get_pwq(int channel, int version);
float get_popularity(int channel, int version);
float get_video_quality(int channel, int version);
float get_video_GHz(int channel, int version);
float get_video_quality_set(int channel, int set);
float get_pwq_set(int channel, int set);
float get_sum_of_version_set_GHz(int channel, int set);*/
int get_backend_max_GHz(int backend_node); 
float get_power(int backend_node, float GHz);

/* CUD-greedy.cpp */
void CUD_greedy(int power_limit);
void non_power_limit_CUP_greedy();
int get_allowable_GHz_CUP_greedy(int backend_node);
float get_total_power_CUP_greedy();
float get_total_GHz_CUP_greedy();

/* VDA-greedy.cpp */
void VDA_greedy(bitrate_version_set_info* _info);
int get_selected_set_VDA_greedy(int channel);
int get_selected_BN_VDA_greedy(int channel, int version);

/* other_methods.cpp */
void other_method(int method_index, int power_limit, bitrate_version_set_info* _info);
void print_method(int method_index);
void set_init_selected_array_other();
void method_RR_HPF(int power_limit);
void method_RR_AP(int power_limit);
void method_RA_HPF(int power_limit);
void method_RA_AP(int power_limit);
void method_PA_HPF(int power_limit);
void method_PA_AP(int power_limit);
//void set_version_set();
