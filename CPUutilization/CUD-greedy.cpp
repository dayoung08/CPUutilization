#include "head.h"

//CUD-greedy.cpp: Power limit 내에서 총 백엔드 노드의 맥시멈 GHz 합이 가장 높도록, 각 백엔드 노드의 맥시멈 GHz를 결정함. 단위는 1

/*List S를 하나 만든다.
이 리스트는 아래의 값들로 처음에 초기화 된다.
처음에는 CPU Utilization이 m%때와, 0%(현재 선택된 j의 Utilization)일 때의 모든 노드에 대한 값 U을 계산한다.
(C^sel_{j,m} - C^sel_{j,0}) / (P_j(m) - P_j(0))
-----
리스트 S는 각 노드 j에 대한 각 U 값 하나 씩을 담고 있다.
그러므로 리스트 S는 0~j개. 0개일때는 모든 노드의 Utiization이 100%로 선택됐을 경우.*/

/*
그런데... 위는 논문에 쓰는 형식이고, 코딩할 때는 CPU 사용율, 즉 %가 아니고 절대적인 값인 GHz 쓴다.
즉 노드 j가 500GHz면 범위는 [0, 500]이고, 각 노드마다 GHz는 다름.
처음부터 그냥 %로 코딩할 수 있도록 할걸.... %로 하면, 노드의 최대 GHz 차이가 많이 날 수록 loss 좀 날거같아서
섬세하게 GHz로 했다가 좀 귀찮아졌다...
*/


int allowable_GHz_CUD_greedy[BN_NUM + 1]; // 각 백엔드 노드에서 사용할 맥시멈 GHz
float total_power_CUD_greedy = 0;
int total_GHz_CUD_greedy = 0;

void CUD_greedy(int power_limit) {
	float power_sum = 0; // 모든 백엔드 노드에서 결정된 GHz일 경우 사용가능한 총 전력 합 실시간 계산용
	//휴리스틱을 위한 값을 생성함
	set<pair<float, pair<int, int>>, greater<pair<float, pair<int, int>>> > set_S;
	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		allowable_GHz_CUD_greedy[backend_node] = 0;
		power_sum += get_power(backend_node, 0); // 모든 백엔드 노드의 사용 가능 GHz이 최대일때의 총 사용 전력의 합
	}

	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		for (int GHz = 1; GHz <= get_backend_max_GHz(backend_node); GHz++) {
			float numerator = GHz;
			float denominator = get_power(backend_node, GHz) - get_power(backend_node, 0);
			float slope = numerator / denominator;
			set_S.insert(make_pair(slope, make_pair(backend_node, GHz))); //현재의 GHz는 0이고, 선택할지 말지 고려할 GHz 가 1일 때.
		}
	}
	//여기까지 수정

	//그리디 형태로 돌림
	while (set_S.size()) {
		int backend_node = (*set_S.begin()).second.first; // slope가 가장 큰 것은 어떤 백엔드 노드인가?
		int GHz = (*set_S.begin()).second.second;
		set_S.erase(set_S.begin());//맨 앞 삭제함
		if (GHz > allowable_GHz_CUD_greedy[backend_node]) {
			float expected_power_sum = power_sum - get_power(backend_node, allowable_GHz_CUD_greedy[backend_node]) + get_power(backend_node, GHz);
			if (expected_power_sum <= power_limit) {
				power_sum = expected_power_sum;
				allowable_GHz_CUD_greedy[backend_node] = GHz;
			}
		}
	}
	//아래 프린트 화면 용 계산
	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		total_GHz_CUD_greedy += allowable_GHz_CUD_greedy[backend_node];
		total_power_CUD_greedy += get_power(backend_node, allowable_GHz_CUD_greedy[backend_node]);
	};
	//total_power = power_sum;

	printf("<<CUD-Greedy>>\n");
	printf("전체 백엔드 노드의 allowable processing capacity 값의 합 : %d GHz\n", total_GHz_CUD_greedy);
	printf("전체 백엔드 노드의 transcoding power 합 : %lf W\n\n", total_power_CUD_greedy);
}

void non_power_limit_CUP_greedy() {
	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		allowable_GHz_CUD_greedy[backend_node] = get_backend_max_GHz(backend_node);
		total_GHz_CUD_greedy += allowable_GHz_CUD_greedy[backend_node];
		total_power_CUD_greedy += get_power(backend_node, allowable_GHz_CUD_greedy[backend_node]);
	}
	printf("<<CUD-Greedy>>\n");
	printf("전체 백엔드 노드의 allowable processing capacity 값의 합 : %d GHz\n", total_GHz_CUD_greedy);
	printf("전체 백엔드 노드의 transcoding power 합 : %lf W\n\n", total_power_CUD_greedy);
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
