#include "head.h"
int main() {
	srand(SEED);
	//int max_power = 1922 * (int)(BN_NUM / BN_TYPE_NUM); //4가지 타입
	int max_power = 7650 * (int)(BN_NUM / BN_TYPE_NUM); //10가지 타입 노드 1개씩 사용율 100%일때의 합 W임
	/*for (int node = 1; node <= (BN_NUM % BN_TYPE_NUM); node++) {
		max_power += get_backend_max_GHz(node);
	}// 8의 배수가 아닐 때*/

	float power_ratio = 0.5; // 디폴트 0.5
	int P_limit = max_power * power_ratio; // 단위는 W 
	int pop_type = MVP;
	int quality_metric = VMAF;
	//이 위의 인자들을 실험 환경에 따라 변경

	printf("===== START =====\n");
	//printf("지프 분포 - 알파 값 : %lf, 베타 값 : %lf\n\n", ALPHA, BETA);
	printf("감마 분포 - k 값 : %lf, 세타 값 : %lf\n\n", K_gamma, THETA_gamma);
	printf("노드 수 : %d, 채널 수 : %d\n\n", BN_NUM, CHANNEL_NUM);
	printf("지정한 P limit : %d W\n", P_limit);

	bitrate_version_set_info bitset_info(0); // 0~4. 0이 디폴트
	segment_init(&bitset_info, pop_type, quality_metric);

	clock_t start, end, spent_time;
	start = clock();
	if (power_ratio == 1) {
		non_power_limit_CUP_greedy();
	}
	else {
		CUD_greedy(P_limit);
	}
	end = clock();
	spent_time = end - start;
	printf("%lf second\n", (float)spent_time / CLOCKS_PER_SEC);

	start = clock();
	VDA_greedy(&bitset_info);
	end = clock();
	spent_time = end - start;
	printf("%lf second\n", (float)spent_time / CLOCKS_PER_SEC);
	
	printf("===== 비교 스킴 =====\n\n");
	other_method(RR_AP, P_limit, &bitset_info);
	other_method(RR_HPF, P_limit, &bitset_info);
	other_method(RA_AP, P_limit, &bitset_info);
	other_method(RA_HPF, P_limit, &bitset_info);
	other_method(PA_AP, P_limit, &bitset_info);
	other_method(PA_HPF, P_limit, &bitset_info);

	printf("===== FINISH =====\n");
}