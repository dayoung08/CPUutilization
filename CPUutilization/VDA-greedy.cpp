#include "head.h"
// VDA-greedy.cpp : 각 채널을 어떤 backend node에 할당할지 결정하고, 각 채널에서 사용할 비트레이트 set을 결정함.

int selected_set[CHANNEL_NUM + 1]; // 각 채널에서 사용하는 트랜스코딩 set
int** selected_BN;//[CHANNEL_NUM + 1][VERSION_NUM]; // 각 채널에서 사용하는 비트레이트 set에 속하는 각 버전이 어떤 BN에서 선택되었는가.
//오리지널 버전은 트랜스코딩 안해서 배열 크기가 저렇다.

bitrate_version_set_info* info;

void VDA_greedy(bitrate_version_set_info* _info) {
	info = _info; set<pair<float, int>, greater<pair<float, int>>> more_remained_GHz_first; // set을 쓰면 자동 정렬이 되어 가장 남은 GHz가 많은 백엔드가 맨 위로 감.
	float more_remained_GHz_first_map[BN_NUM+1]; // set으로는 GHz 변경에 따른 update가 좀 복잡해서 따로 map도 선언해서 update를 도움.


	selected_BN = (int**)malloc(sizeof(int*) * (CHANNEL_NUM + 1));
	for (int row = 1; row <= CHANNEL_NUM; row++) {
		selected_BN[row] = (int*)malloc(sizeof(int) * (info->version_num));  // 오리지널 버전은 트랜스코딩 안하니까
		for (int col = 1; col < info->version_num; col++) {  // 오리지널 버전은 트랜스코딩 안하니까
			selected_BN[row][col] = 0;
		}
	}

	//1. initialization
	//백엔드 노드의 남은 GHz를 최대 GHz로 초기화 해줌

	float GHz_limit = 0;
	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		int temp;
		temp = get_allowable_GHz_CUP_greedy(backend_node);
		more_remained_GHz_first.insert(make_pair(temp, backend_node)); //set
		more_remained_GHz_first_map[backend_node] = temp; //map
		GHz_limit += temp;
	}

	// 각 채널들을 가장 높은 set으로 설정
	float GHz_sum = 0;
	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		selected_set[channel] = info->version_set_num;
		GHz_sum += info->sum_of_version_set_GHz[channel][info->version_set_num];
	}

	//2 set 선택 step.
	//휴리스틱을 위한 값을 생성함
	set<pair<float, pair<int, int>>, less<pair<float, pair<int, int>>> > set_A;
	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		for (int set = 1; set <= info->version_set_num - 1; set++) { //소스는 전부 1080p
			float slope = (info->pwq_set[channel][info->version_set_num] - info->pwq_set[channel][set]) / (info->sum_of_version_set_GHz[channel][info->version_set_num] - info->sum_of_version_set_GHz[channel][set]);
			set_A.insert(make_pair(slope, make_pair(channel, set)));
		}
	}

	//그리디 형태로 돌림
	while (set_A.size()) {
		int channel = (*set_A.begin()).second.first; // slope가 가장 큰 것은 어떤 채널인가?
		int set = (*set_A.begin()).second.second; //slope가 가장 큰 것은 어떤 세트인가?

		set_A.erase(set_A.begin());//맨 앞 삭제함

		//int prev_backend_node = selected_BN[channel];
		int prev_set = selected_set[channel];
		if (info->sum_of_version_set_GHz[channel][set] < info->sum_of_version_set_GHz[channel][prev_set]) {
			float expected_GHz_sum = GHz_sum - info->sum_of_version_set_GHz[channel][prev_set] + info->sum_of_version_set_GHz[channel][set];
			GHz_sum = expected_GHz_sum;
			selected_set[channel] = set;
			if (expected_GHz_sum < GHz_limit) {
				break;
			}
		}
	}
	float GHz_sum_temp = 0;
	float pwq_sum_temp = 0;
	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		GHz_sum_temp += info->sum_of_version_set_GHz[channel][selected_set[channel]];
		pwq_sum_temp += info->pwq_set[channel][selected_set[channel]];
	}
	std::printf("=중간과정= GHz_sum_temp : %lf GHz, pwq_sum_temp : %lf\n", GHz_sum_temp, pwq_sum_temp);
	std::printf("=모든 백엔드의 총 합 GHz : %lf GHz\n\n", GHz_limit);

	//각 set의 버전들을 set B에 삽입함.
	//3 백엔드 노드 할당 step.
	//휴리스틱을 위한 값을 생성함
	//아래는 1번 버전을 할당했을 때 문제가 발생하는지 확인 하기 위함. 
	
	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		int backend_node = (*more_remained_GHz_first.begin()).second; // 가장 남은 GHz가 많은 백엔드는 무엇인가?
		float backend_node_GHz = (*more_remained_GHz_first.begin()).first; // 그 백엔드의 GHz는 얼마인가?

		selected_BN[channel][1] = backend_node;

		if (backend_node_GHz - info->video_GHz[channel][1] < 0) {
			std::printf("\n==(1) P limit 값이 너무 적거나\n(2)해당 코드의 노드와 채널 수 세팅에 문제가 있음. 노드가 너무 적거나 채널이 너무 많음==\n");
			exit(0);
		}

		more_remained_GHz_first.erase(more_remained_GHz_first.begin());
		more_remained_GHz_first.insert(make_pair((backend_node_GHz - info->video_GHz[channel][1]), backend_node)); //set 갱신
		more_remained_GHz_first_map[backend_node] = (backend_node_GHz - info->video_GHz[channel][1]); //map 갱신
	}

	//휴리스틱을 위한 값을 생성함
	set<pair<long float, pair<int, int>>, greater<pair<long float, pair<int, int>>> > set_B;
	//int cnt = 0;
	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		int set = selected_set[channel];
		for (int version = 1; version <= info->version_num - 1; version++) { 
			if ((set - 1) & (info->number_for_bit_opration >> (info->set_versions_number_for_bit_opration - (version - 1)))) { // 이전에 선택한 set에서 할당했던 GHz는 전부 삭제해 준다. 
				float slope = (info->pwq[channel][version] / (info->video_GHz[channel][version]));
				set_B.insert(make_pair(slope, make_pair(channel, version)));
				//cnt++;
			}
		}
	}

	while (set_B.size()) {
		int channel = (*set_B.begin()).second.first; // slope가 가장 큰 것은 어떤 채널인가?
		int version = (*set_B.begin()).second.second; //slope가 가장 큰 것은 어떤 버전인가?

		set_B.erase(set_B.begin());//맨 앞 삭제함

		float GHz = (*more_remained_GHz_first.begin()).first;
		int selected_bk = (*more_remained_GHz_first.begin()).second; // 가장 남은 GHz가 많은 백엔드는 무엇인가?

		float cal = (GHz - info->video_GHz[channel][version]);
		if (cal >= 0) { // 선택된 노드가 아직 꽉 차지 않았다면
			selected_BN[channel][version] = selected_bk;
			more_remained_GHz_first.erase(*more_remained_GHz_first.begin());
			more_remained_GHz_first.insert(make_pair(GHz - info->video_GHz[channel][version], selected_bk)); //새로 선택된 백엔드 노드에 할당
			more_remained_GHz_first_map[selected_bk] = (more_remained_GHz_first_map[selected_bk] - info->video_GHz[channel][version]); //map 갱신
		}
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		int set = 1;
		for (int version = 2; version <= info->version_num - 1; version++) {
			if (selected_BN[channel][version] != 0)
				set += (info->number_for_bit_opration >> (info->set_versions_number_for_bit_opration - (version - 1)));
		}
		selected_set[channel] = set;
	}

	//finalization은 알고리즘 상에서 temp값 -> 진짜 값 확정하는 과정이라 여기선 필요 x
	//but 아래는 각 노드 마다 할당된 채널들을 정리하려고 하는 과정.

	//vector<pair<int,int>> channel_list[BN_NUM+1];
	float pwq_sum = 0;
	float video_quality_sum = 0;
	float pwq_sum_range[10]; // 90~100%, 80~90% ... 0~10%까지의 Average PWQ를 비교.
	float video_quality_sum_range[10]; // 90~100%, 80~90% ... 0~10%까지의 Average video_quality을 비교.
	float used_GHz[BN_NUM + 1];

	for (int per_index = 0; per_index < 10; per_index++) {
		video_quality_sum_range[per_index] = 0;
		pwq_sum_range[per_index] = 0;
	}

	int* number_of_transcoding = (int*)malloc(sizeof(int) * (info->version_num));
	for (int version = 1; version <= info->version_num - 1; version++) {
		number_of_transcoding[version] = 0;
	}
	// 각 비트레이트 버전 중 실제 transcoding된 갯수 계산 용. 이 갯수의 range는 [0, CHANNEL_NUM]

	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		used_GHz[backend_node] = 0;
	}
	std::printf("<<VDA-Greedy>>\n");
	//printf("[채널-세트]\n");

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		pwq_sum += info->pwq_set[channel][selected_set[channel]];
		video_quality_sum += info->video_quality_set[channel][selected_set[channel]];
		for (int version = 1; version <= info->version_num - 1; version++) {
			if (((selected_set[channel] - 1) & info->number_for_bit_opration >> (info->set_versions_number_for_bit_opration - (version - 1)) || (version == 1))) {
				number_of_transcoding[version]++;

				int backend_node = selected_BN[channel][version];
				//printf("channel - version : %d - %d : %d\n", channel, version, backend_node);
				used_GHz[backend_node] += info->video_GHz[channel][version];
			}
		}
		//std::printf("%d\n", (channel - 1) / (CHANNEL_NUM / 10));
		video_quality_sum_range[(channel - 1) / (CHANNEL_NUM / 10)] += info->video_quality_set[channel][selected_set[channel]]; // 90~100%, 80~90% ... 0~10%까지의 Average video_quality을 비교.
		pwq_sum_range[(channel - 1) / (CHANNEL_NUM / 10)] += info->pwq_set[channel][selected_set[channel]]; // 90~100%, 80~90% ... 0~10%까지의 Average video_quality을 비교.

	}
	std::printf("\n전체 pwq의 합 : %lf", pwq_sum);
	// 이게 차이가 중간과정에서 나온 pwq와 차이가 없게 출력될 때 그 이유는, backend 노드 할당을 못한 버전은 매우 작은 pwq를 가졌기 때문이다. 즉 차이는 있는데 소숫점 짤렸다. 
	std::printf("\n전체 video_quality의 평균 : %lf\n\n", video_quality_sum / CHANNEL_NUM / info->version_num);
	/*for (int per_index = 0; per_index < 10; per_index++) {
		printf("%d%~%d%% pwq의 합/video_quality의 평균 : %lf, %lf\n", (per_index * 10), (per_index + 1) * 10, pwq_sum_range[per_index], video_quality_sum_range[per_index] / (CHANNEL_NUM / 10) / info->version_num);
	}*/

	int power_sum_final = 0;
	float GHz_sum_final = 0;
	//selected_BN
	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		power_sum_final += get_power(backend_node, used_GHz[backend_node]); // 전체에서 남은 걸 빼면 사용한 GHz
		GHz_sum_final += used_GHz[backend_node];
	}
	std::printf("\n사용 전력 : %d W\n", power_sum_final);
	std::printf("사용 GHz : %lf GHz\n\n", GHz_sum_final);

	for (int version = info->version_num - 1; version >= 2; version--) {
		printf("버전 %d : %d\n", version, number_of_transcoding[version]);
	}
	printf("\n");
}

int get_selected_set_greedy(int channel) {
	return selected_set[channel];
}

int get_selected_BN_greedy(int channel, int version) {
	return selected_BN[channel][version];
}
