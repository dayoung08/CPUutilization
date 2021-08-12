#include "head.h"

//CUD-greedy.cpp: Power limit ������ �� �鿣�� ����� �ƽø� GHz ���� ���� ������, �� �鿣�� ����� �ƽø� GHz�� ������. ������ 1

/*List S�� �ϳ� �����.
�� ����Ʈ�� �Ʒ��� ����� ó���� �ʱ�ȭ �ȴ�.
ó������ CPU Utilization�� m%����, 0%(���� ���õ� j�� Utilization)�� ���� ��� ��忡 ���� �� U�� ����Ѵ�.
(C^sel_{j,m} - C^sel_{j,0}) / (P_j(m) - P_j(0))
-----
����Ʈ S�� �� ��� j�� ���� �� U �� �ϳ� ���� ��� �ִ�.
�׷��Ƿ� ����Ʈ S�� 0~j��. 0���϶��� ��� ����� Utiization�� 100%�� ���õ��� ���.*/

/*
�׷���... ���� ���� ���� �����̰�, �ڵ��� ���� CPU �����, �� %�� �ƴϰ� �������� ���� GHz ����.
�� ��� j�� 500GHz�� ������ [0, 500]�̰�, �� ��帶�� GHz�� �ٸ�.
ó������ �׳� %�� �ڵ��� �� �ֵ��� �Ұ�.... %�� �ϸ�, ����� �ִ� GHz ���̰� ���� �� ���� loss �� ���Ű��Ƽ�
�����ϰ� GHz�� �ߴٰ� �� ����������...
*/


int allowable_GHz_CUD_greedy[BN_NUM + 1]; // �� �鿣�� ��忡�� ����� �ƽø� GHz
float total_power_CUD_greedy = 0;
int total_GHz_CUD_greedy = 0;

void CUD_greedy(int power_limit) {
	float power_sum = 0; // ��� �鿣�� ��忡�� ������ GHz�� ��� ��밡���� �� ���� �� �ǽð� ����
	//�޸���ƽ�� ���� ���� ������
	set<pair<float, pair<int, int>>, greater<pair<float, pair<int, int>>> > set_S;
	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		allowable_GHz_CUD_greedy[backend_node] = 0;
		power_sum += get_power(backend_node, 0); // ��� �鿣�� ����� ��� ���� GHz�� �ִ��϶��� �� ��� ������ ��
	}

	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		for (int GHz = 1; GHz <= get_backend_max_GHz(backend_node); GHz++) {
			float numerator = GHz;
			float denominator = get_power(backend_node, GHz) - get_power(backend_node, 0);
			float slope = numerator / denominator;
			set_S.insert(make_pair(slope, make_pair(backend_node, GHz))); //������ GHz�� 0�̰�, �������� ���� ����� GHz �� 1�� ��.
		}
	}
	//������� ����

	//�׸��� ���·� ����
	while (set_S.size()) {
		int backend_node = (*set_S.begin()).second.first; // slope�� ���� ū ���� � �鿣�� ����ΰ�?
		int GHz = (*set_S.begin()).second.second;
		set_S.erase(set_S.begin());//�� �� ������
		if (GHz > allowable_GHz_CUD_greedy[backend_node]) {
			float expected_power_sum = power_sum - get_power(backend_node, allowable_GHz_CUD_greedy[backend_node]) + get_power(backend_node, GHz);
			if (expected_power_sum <= power_limit) {
				power_sum = expected_power_sum;
				allowable_GHz_CUD_greedy[backend_node] = GHz;
			}
		}
	}
	//�Ʒ� ����Ʈ ȭ�� �� ���
	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		total_GHz_CUD_greedy += allowable_GHz_CUD_greedy[backend_node];
		total_power_CUD_greedy += get_power(backend_node, allowable_GHz_CUD_greedy[backend_node]);
	};
	//total_power = power_sum;

	printf("<<CUD-Greedy>>\n");
	printf("��ü �鿣�� ����� allowable processing capacity ���� �� : %d GHz\n", total_GHz_CUD_greedy);
	printf("��ü �鿣�� ����� transcoding power �� : %lf W\n\n", total_power_CUD_greedy);
}

void non_power_limit_CUP_greedy() {
	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		allowable_GHz_CUD_greedy[backend_node] = get_backend_max_GHz(backend_node);
		total_GHz_CUD_greedy += allowable_GHz_CUD_greedy[backend_node];
		total_power_CUD_greedy += get_power(backend_node, allowable_GHz_CUD_greedy[backend_node]);
	}
	printf("<<CUD-Greedy>>\n");
	printf("��ü �鿣�� ����� allowable processing capacity ���� �� : %d GHz\n", total_GHz_CUD_greedy);
	printf("��ü �鿣�� ����� transcoding power �� : %lf W\n\n", total_power_CUD_greedy);
}

int get_allowable_GHz_CUP_greedy(int backend_node) {
	return allowable_GHz_CUD_greedy[backend_node];
}

float get_total_power_CUP_greedy() {
	return total_power_CUD_greedy;
}
float get_total_GHz_CUP_greedy() {
	return total_GHz_CUD_greedy;
}
