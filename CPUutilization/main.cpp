#include "head.h"
int main() {
	srand(SEED);
	//int max_power = 1922 * (int)(BN_NUM / BN_TYPE_NUM); //4���� Ÿ��
	int max_power = 7650 * (int)(BN_NUM / BN_TYPE_NUM); //10���� Ÿ�� ��� 1���� ����� 100%�϶��� �� W��
	/*for (int node = 1; node <= (BN_NUM % BN_TYPE_NUM); node++) {
		max_power += get_backend_max_GHz(node);
	}// 8�� ����� �ƴ� ��*/

	float power_ratio = 0.5; // ����Ʈ 0.5
	int P_limit = max_power * power_ratio; // ������ W 
	int pop_type = MVP;
	int quality_metric = VMAF;
	//�� ���� ���ڵ��� ���� ȯ�濡 ���� ����

	printf("===== START =====\n");
	//printf("���� ���� - ���� �� : %lf, ��Ÿ �� : %lf\n\n", ALPHA, BETA);
	printf("���� ���� - k �� : %lf, ��Ÿ �� : %lf\n\n", K_gamma, THETA_gamma);
	printf("��� �� : %d, ä�� �� : %d\n\n", BN_NUM, CHANNEL_NUM);
	printf("������ P limit : %d W\n", P_limit);

	bitrate_version_set_info bitset_info(0); // 0~4. 0�� ����Ʈ
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
	
	printf("===== �� ��Ŵ =====\n\n");
	other_method(RR_AP, P_limit, &bitset_info);
	other_method(RR_HPF, P_limit, &bitset_info);
	other_method(RA_AP, P_limit, &bitset_info);
	other_method(RA_HPF, P_limit, &bitset_info);
	other_method(PA_AP, P_limit, &bitset_info);
	other_method(PA_HPF, P_limit, &bitset_info);

	printf("===== FINISH =====\n");
}