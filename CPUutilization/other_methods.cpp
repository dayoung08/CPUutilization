#include "head.h"

//비교 스킴들을 짜기 위한 코드이다.
short selected_set_other[CHANNEL_NUM + 1]; // 각 채널에서 사용하는 비트레이트 set
short** selected_BN_other;//[CHANNEL_NUM + 1][VERSION_NUM]; // 각 채널에서 사용하는 비트레이트 set에 속하는 각 버전이 어떤 BN에서 선택되었는가.
//오리지널 버전은 트랜스코딩 안해서 배열 크기가 저렇다.

float remained_GHz[BN_NUM + 1];//모든 노드의 남은 Hz 계산을 위해
bitrate_version_set_info* info_other;

void other_method(int method_index, int power_limit, bitrate_version_set_info* _info) {
	info_other = _info;

	selected_BN_other = (short**)malloc(sizeof(short*) * (CHANNEL_NUM + 1));
	for (int row = 1; row <= CHANNEL_NUM; row++) {
		selected_BN_other[row] = (short*)malloc(sizeof(short) * (info_other->version_num));  // 오리지널 버전은 트랜스코딩 안하니까
		for (int col = 1; col < info_other->version_num; col++) {  // 오리지널 버전은 트랜스코딩 안하니까
			selected_BN_other[row][col] = 0;
		}
	}

	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		remained_GHz[backend_node] = get_backend_max_GHz(backend_node);
	}
	set_init_selected_array_other();
	if (method_index == RR_AP) {
		method_RR_AP(power_limit);
	}
	else if (method_index == RR_HPF) {
		method_RR_HPF(power_limit);
	}
	else if (method_index == RA_AP) {
		method_RA_AP(power_limit);
	}
	else if (method_index == RA_HPF) {
		method_RA_HPF(power_limit);
	}
	else if (method_index == PA_AP) {
		method_PA_AP(power_limit);
	}
	else if (method_index == PA_HPF) {
		method_PA_HPF(power_limit);
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		int set = 1;
		for (int version = 2; version <= info_other->version_num - 1; version++) {
			if (selected_BN_other[channel][version] != 0)
				set += (info_other->number_for_bit_opration >> (info_other->set_versions_number_for_bit_opration - (version - 1)));
		}
		selected_set_other[channel] = set;
	}
	print_method(method_index);
}

void print_method(int method_index) {
	if (method_index == RR_AP) {
		printf("<<RR_AP>>\n");
	}
	else if (method_index == RR_HPF) {
		printf("<<RR_HPF>>\n");
	}
	else if (method_index == RA_AP) {
		printf("<<RA_AP>>\n");
	}
	else if (method_index == RA_HPF) {
		printf("<<RA_HPF>>\n");
	}
	else if (method_index == PA_AP) {
		printf("<<PA_AP>>\n");
	}
	else if (method_index == PA_HPF) {
		printf("<<PA_HPF>>\n");
	}
	float pwq_sum = 0;
	float video_quality_sum = 0;
	float pwq_sum_range[10]; // 90~100%, 80~90% ... 0~10%까지의 Average PWQ를 비교.
	float video_quality_sum_range[10]; // 90~100%, 80~90% ... 0~10%까지의 Average video_quality을 비교.
	for (int per_index = 0; per_index < 10; per_index++) {
		video_quality_sum_range[per_index] = 0;
		pwq_sum_range[per_index] = 0;
	}

	int* number_of_transcoding = (int*)malloc(sizeof(int) * (info_other->version_num));
	for (int version = 1; version <= info_other->version_num - 1; version++) {
		number_of_transcoding[version] = 0;
	}
	// 각 비트레이트 버전 중 실제 transcoding된 갯수 계산 용. 이 갯수의 range는 [0, CHANNEL_NUM]

	//printf("[채널-세트]\n");
	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		/*if (info_other->pwq_set[channel][selected_set_other[channel]] < 0) {
			cout << "디버그";
		}*/
		pwq_sum += info_other->pwq_set[channel][selected_set_other[channel]];
		video_quality_sum += info_other->video_quality_set[channel][selected_set_other[channel]];
		for (int version = 1; version <= info_other->version_num - 1; version++) {
			if (((selected_set_other[channel] - 1) & info_other->version_set_num >> ((info_other->version_num - 2) - (version - 1)) || (version == 1))) {
				number_of_transcoding[version]++;
			}
		}
		video_quality_sum_range[(channel - 1) / (CHANNEL_NUM / 10)] += info_other->video_quality_set[channel][selected_set_other[channel]]; // 90~100%, 80~90% ... 0~10%까지의 Average video_quality을 비교.
		pwq_sum_range[(channel - 1) / (CHANNEL_NUM / 10)] += info_other->pwq_set[channel][selected_set_other[channel]]; // 90~100%, 80~90% ... 0~10%까지의 Average video_quality을 비교.
	}
	printf("\n전체 pwq의 합 : %lf", pwq_sum);
	printf("\n전체 video_quality의 평균 : %lf\n\n", video_quality_sum / CHANNEL_NUM / info_other->version_num);
	/*for (int per_index = 0; per_index < 10; per_index++) {
		printf("%d%~%d%% pwq의 합/video_quality의 평균 : %lf, %lf\n", (per_index * 10), (per_index + 1) * 10, pwq_sum_range[per_index], video_quality_sum_range[per_index] / (CHANNEL_NUM / 10) / info_other->version_num);
	}*/

	int power_sum = 0;
	float util_sum = 0;
	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		//printf("[백엔드 노드 %d]\n", backend_node);
		//printf("남은 GHz, 최대 GHz : %lf / %d\n", remained_GHz[backend_node], get_backend_max_GHz(backend_node));
		float util = get_backend_max_GHz(backend_node) - remained_GHz[backend_node];
		power_sum += get_power(backend_node, util); // 전체에서 남은 걸 빼면 사용한 GHz
		util_sum += util;
	}
	printf("\n사용 전력 : %d W\n", power_sum);
	printf("사용 GHz : %lf GHz\n\n", util_sum);

	for (int version = info_other->version_num - 1; version >= 2; version--) {
		printf("버전 %d : %d\n", version, number_of_transcoding[version]);
	}
	printf("\n");
}

void set_init_selected_array_other() {
	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		selected_set_other[channel] = 0;
		for (int version = 2; version <= info_other->version_num - 1; version++) {
			selected_BN_other[channel][version] = 0;
		}
	}
}


void method_RR_AP(int power_limit) {
	bool flag = true;
	int backend_node = 0; //0에서 시작하는거지 0이 백엔드 노드 인덱스가 되는게 아님
	set<pair<float, int>, greater<pair<float, int>>> high_pop_channel_first;

	bool is_full[BN_NUM + 1];
	for (int bk = 1; bk <= BN_NUM; bk++) {
		is_full[bk] = false;
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		do {
			backend_node++; //RR로 backend node 할당하기 위해 ++
			if (backend_node > BN_NUM) {
				backend_node = 1;
			}

			if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0)) {
				if (!is_full[backend_node]) {
					is_full[backend_node] = true;
				}
				int full_node_cnt = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (is_full[bk])
						full_node_cnt++;
				}
				if (full_node_cnt >= BN_NUM) { // 모든 백엔드가 꽉 차있다면
					flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
					break;
				}
			}
		} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0));// 해당 노드가 꽉 찼는지 아닌지 확인

		if (flag) {
			float power_sum = 0;
			for (int bk = 1; bk <= BN_NUM; bk++) {
				if (backend_node == bk)
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][1]));
				else
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
			}//전력이 꽉 찼는지 아닌지 확인
			if (power_sum > power_limit) {
				flag = false; //이제 할당 중지한다.
				//printf("1. RR_AP() : 1번 다 못했는데 꽉 참\n");
				break;
			}

			remained_GHz[backend_node] -= info_other->video_GHz[channel][1]; // 1번 버전을 라운드 로빈 형태로 할당함.
			selected_BN_other[channel][1] = backend_node;
		}

		high_pop_channel_first.insert(make_pair(info_other->get_channel_popularity(channel), channel));
	}

	int high_channel_cnt = 0;
	while (high_pop_channel_first.size() && flag) { //그 다음 2번 버전~ 6번 버전은 pop 순으로 라운드 로빈으로 할당함.
		int channel = (*high_pop_channel_first.begin()).second; //가장 높은 pop인 채널
		high_pop_channel_first.erase(*high_pop_channel_first.begin()); // 맨 위의 값 삭제

		for (int version = 2; version <= info_other->version_num - 1; version++) {
			do {
				backend_node++; //RR로 backend node 할당하기 위해 ++
				if (backend_node > BN_NUM) {
					backend_node = 1;
				}

				if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0)) {
					if (!is_full[backend_node]) {
						is_full[backend_node] = true;
					}
					int full_node_cnt = 0;
					for (int bk = 1; bk <= BN_NUM; bk++) {
						if (is_full[bk])
							full_node_cnt++;
					}
					if (full_node_cnt >= BN_NUM) { // 모든 백엔드가 꽉 차있다면
						flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
						//현재 version 이전거 ~ 2번까지 할당한 걸 삭제한다.
						for (int ver = (version - 1); ver >= 2; ver--) {
							remained_GHz[selected_BN_other[channel][ver]] += info_other->video_GHz[channel][ver];
							selected_BN_other[channel][ver] = 0;
						}
						break;
					}
				}
			} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0));// 해당 노드가 꽉 찼는지 아닌지 확인

			if (flag) {
				float power_sum = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (backend_node == bk)
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][version]));
					else
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
				}//전력이 꽉 찼는지 아닌지 확인
				if (power_sum > power_limit) {
					//현재 version 이전거 ~ 2번까지 할당한 걸 삭제한다.
					for (int ver = (version - 1); ver >= 2; ver--) {
						remained_GHz[selected_BN_other[channel][ver]] += info_other->video_GHz[channel][ver];
						selected_BN_other[channel][ver] = 0;
					}

					flag = false; //이제 할당 중지한다.
					break;
				}

				remained_GHz[backend_node] -= info_other->video_GHz[channel][version]; // 나머지 버전을 라운드 로빈 형태로 할당함.
				selected_BN_other[channel][version] = backend_node;
			}
		}
	}
}

void method_RR_HPF(int power_limit) {
	bool flag = true;
	int backend_node = 0; //0에서 시작하는거지 0이 백엔드 노드 인덱스가 되는게 아님

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		bool is_full[BN_NUM + 1];
		for (int bk = 1; bk <= BN_NUM; bk++) {
			is_full[bk] = false;
		}
		do {
			backend_node++; //RR로 backend node 할당하기 위해 ++
			if (backend_node > BN_NUM) {
				backend_node = 1;
			}

			if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0)) {
				if (!is_full[backend_node]) {
					is_full[backend_node] = true;
				}
				int full_node_cnt = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (is_full[bk])
						full_node_cnt++;
				}
				if (full_node_cnt >= BN_NUM) { // 모든 백엔드가 꽉 차있다면
					flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
					break;
				}
			}
		} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0));// 해당 노드가 꽉 찼는지 아닌지 확인

		if (flag) {
			float power_sum = 0;
			for (int bk = 1; bk <= BN_NUM; bk++) {
				if (backend_node == bk)
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][1]));
				else
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
			}//전력이 꽉 찼는지 아닌지 확인
			if (power_sum > power_limit) {
				flag = false; //이제 할당 중지한다.
				//printf("1. RR_AP() : 1번 다 못했는데 꽉 참\n");
				break;
			}

			remained_GHz[backend_node] -= info_other->video_GHz[channel][1]; // 1번 버전을 라운드 로빈 형태로 할당함.
			selected_BN_other[channel][1] = backend_node;
		}
	}

	set<pair<float, int>, greater<pair<float, int>> > high_pop_version_first;  //가장 평균 pop이 높은 버전을 찾기 위함
	float* version_pop_average = (float*)malloc(sizeof(float) * (info_other->version_num)); // 버전 1은 이미 트랜스코딩했고 마지막버전은 원본이라 트랜스코딩 안함
	for (int version = 2; version <= info_other->version_num - 1; version++) {
		version_pop_average[version] = 0;
		for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
			version_pop_average[version] += info_other->popularity[channel][version];
		}
		version_pop_average[version] /= CHANNEL_NUM;
	}
	for (int version = 2; version <= info_other->version_num - 1; version++) {
		high_pop_version_first.insert(make_pair(version_pop_average[version], version));
	}
	//여기까지 세팅

	while (high_pop_version_first.size() && flag) {
		int version = (*high_pop_version_first.begin()).second; // 가장 높은 pop인 버전
		high_pop_version_first.erase(*high_pop_version_first.begin()); // 맨 위의 값 삭제
		for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
			bool is_full[BN_NUM + 1];
			for (int bk = 1; bk <= BN_NUM; bk++) {
				is_full[bk] = false;
			}
			do {
				backend_node++; //RR로 backend node 할당하기 위해 ++
				if (backend_node > BN_NUM) {
					backend_node = 1;
				}

				if ((remained_GHz[backend_node] - info_other->video_GHz[channel][version]) < 0) {
					if (!is_full[backend_node]) {
						is_full[backend_node] = true;
					}
					int full_node_cnt = 0;
					for (int bk = 1; bk <= BN_NUM; bk++) {
						if (is_full[bk])
							full_node_cnt++;
					}
					if (full_node_cnt >= BN_NUM) { // 모든 백엔드가 꽉 차있다면
						flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
						break;
					}
				}
			} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version]) < 0);// 해당 노드가 꽉 찼는지 아닌지 확인

			if (flag) {
				float power_sum = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (backend_node == bk)
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][version]));
					else
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
				}//전력이 꽉 찼는지 아닌지 확인
				if (power_sum > power_limit) {
					flag = false; //이제 할당 중지한다.
					break;
				}

				remained_GHz[backend_node] -= info_other->video_GHz[channel][version]; // 나머지 버전을 라운드 로빈 형태로 할당함.
				selected_BN_other[channel][version] = backend_node;
			}
		}
	}
}


void method_RA_AP(int power_limit) {
	bool flag = true;
	int backend_node = 0; //0에서 시작하는거지 0이 백엔드 노드 인덱스가 되는게 아님
	set<pair<float, int>, greater<pair<float, int>>> high_pop_channel_first;

	bool is_full[BN_NUM + 1];
	for (int bk = 1; bk <= BN_NUM; bk++) {
		is_full[bk] = false;
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		do {
			backend_node = rand() % BN_NUM + 1; //랜덤으로 백엔드 노드 할당

			if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0)) {
				if (!is_full[backend_node]) {
					is_full[backend_node] = true;
				}
				int full_node_cnt = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (is_full[bk])
						full_node_cnt++;
				}
				if (full_node_cnt >= BN_NUM) { // 모든 백엔드가 꽉 차있다면
					flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
					break;
				}
			}
		} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0));// 해당 노드가 꽉 찼는지 아닌지 확인

		if (flag) {
			float power_sum = 0;
			for (int bk = 1; bk <= BN_NUM; bk++) {
				if (backend_node == bk)
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][1]));
				else
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
			}//전력이 꽉 찼는지 아닌지 확인
			if (power_sum > power_limit) {
				flag = false; //이제 할당 중지한다.
				//printf("1. RR_AP() : 1번 다 못했는데 꽉 참\n");
				break;
			}

			remained_GHz[backend_node] -= info_other->video_GHz[channel][1]; // 1번 버전을 라운드 로빈 형태로 할당함.
			selected_BN_other[channel][1] = backend_node;
		}

		high_pop_channel_first.insert(make_pair(info_other->get_channel_popularity(channel), channel));
	}

	int high_channel_cnt = 0;
	while (high_pop_channel_first.size() && flag) { //그 다음 2번 버전~ 6번 버전은 pop 순으로 라운드 로빈으로 할당함.
		int channel = (*high_pop_channel_first.begin()).second; //가장 높은 pop인 채널
		high_pop_channel_first.erase(*high_pop_channel_first.begin()); // 맨 위의 값 삭제

		for (int version = 2; version <= info_other->version_num - 1; version++) {
			do {
				backend_node = rand() % BN_NUM + 1; //랜덤으로 백엔드 노드 할당

				if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0)) {
					if (!is_full[backend_node]) {
						is_full[backend_node] = true;
					}
					int full_node_cnt = 0;
					for (int bk = 1; bk <= BN_NUM; bk++) {
						if (is_full[bk])
							full_node_cnt++;
					}
					if (full_node_cnt >= BN_NUM) { // 모든 백엔드가 꽉 차있다면
						flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
						//현재 version 이전거 ~ 2번까지 할당한 걸 삭제한다.
						for (int ver = (version - 1); ver >= 2; ver--) {
							remained_GHz[selected_BN_other[channel][ver]] += info_other->video_GHz[channel][ver];
							selected_BN_other[channel][ver] = 0;
						}
						break;
					}
				}
			} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0));// 해당 노드가 꽉 찼는지 아닌지 확인

			if (flag) {
				float power_sum = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (backend_node == bk)
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][version]));
					else
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
				}//전력이 꽉 찼는지 아닌지 확인
				if (power_sum > power_limit) {
					//현재 version 이전거 ~ 2번까지 할당한 걸 삭제한다.
					for (int ver = (version - 1); ver >= 2; ver--) {
						remained_GHz[selected_BN_other[channel][ver]] += info_other->video_GHz[channel][ver];
						selected_BN_other[channel][ver] = 0;
					}

					flag = false; //이제 할당 중지한다.
					break;
				}

				remained_GHz[backend_node] -= info_other->video_GHz[channel][version]; // 나머지 버전을 라운드 로빈 형태로 할당함.
				selected_BN_other[channel][version] = backend_node;
			}
		}
	}
}

void method_RA_HPF(int power_limit) {
	bool flag = true;
	int backend_node = 0; //0에서 시작하는거지 0이 백엔드 노드 인덱스가 되는게 아님

	bool is_full[BN_NUM + 1];
	for (int bk = 1; bk <= BN_NUM; bk++) {
		is_full[bk] = false;
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		int full_node_cnt = 0;
		do {
			backend_node = rand() % BN_NUM + 1; //랜덤으로 백엔드 노드 할당

			if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0)) {
				if (!is_full[backend_node]) {
					is_full[backend_node] = true;
				}
				int full_node_cnt = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (is_full[bk])
						full_node_cnt++;
				}
				if (full_node_cnt >= BN_NUM) { // 모든 백엔드가 꽉 차있다면
					flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
					//현재 version 이전거 ~ 2번까지 할당한 걸 삭제한다.
					break;
				}
			}
		} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0));// 해당 노드가 꽉 찼는지 아닌지 확인

		if (flag) {
			float power_sum = 0;
			for (int bk = 1; bk <= BN_NUM; bk++) {
				if (backend_node == bk)
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][1]));
				else
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
			}//전력이 꽉 찼는지 아닌지 확인
			if (power_sum > power_limit) {
				flag = false; //이제 할당 중지한다.
				//printf("1. RR_AP() : 1번 다 못했는데 꽉 참\n");
				break;
			}

			remained_GHz[backend_node] -= info_other->video_GHz[channel][1]; // 1번 버전을 라운드 로빈 형태로 할당함.
			selected_BN_other[channel][1] = backend_node;
		}
	}

	set<pair<float, int>, greater<pair<float, int>> > high_pop_version_first;  //가장 평균 pop이 높은 버전을 찾기 위함
	float* version_pop_average = (float*)malloc(sizeof(float) * (info_other->version_num)); // 버전 1은 이미 트랜스코딩했고 마지막버전은 원본이라 트랜스코딩 안함
	for (int version = 2; version <= info_other->version_num - 1; version++) {
		version_pop_average[version] = 0;
		for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
			version_pop_average[version] += info_other->popularity[channel][version];
		}
		version_pop_average[version] /= CHANNEL_NUM;
	}
	for (int version = 2; version <= info_other->version_num - 1; version++) {
		high_pop_version_first.insert(make_pair(version_pop_average[version], version));
	}
	//여기까지 세팅

	while (high_pop_version_first.size() && flag) {
		int version = (*high_pop_version_first.begin()).second; // 가장 높은 pop인 버전
		high_pop_version_first.erase(*high_pop_version_first.begin()); // 맨 위의 값 삭제
		for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
			do {
				backend_node = rand() % BN_NUM + 1; //랜덤으로 백엔드 노드 할당

				if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0)) {
					if (!is_full[backend_node]) {
						is_full[backend_node] = true;
					}
					int full_node_cnt = 0;
					for (int bk = 1; bk <= BN_NUM; bk++) {
						if (is_full[bk])
							full_node_cnt++;
					}
					if (full_node_cnt >= BN_NUM) { // 모든 백엔드가 꽉 차있다면
						flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
						break;
					}
				}
			} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0));// 해당 노드가 꽉 찼는지 아닌지 확인

			if (flag) {
				float power_sum = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (backend_node == bk)
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][version]));
					else
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
				}//전력이 꽉 찼는지 아닌지 확인
				if (power_sum > power_limit) {
					flag = false; //이제 할당 중지한다.
					break;
				}

				remained_GHz[backend_node] -= info_other->video_GHz[channel][version]; // 나머지 버전을 라운드 로빈 형태로 할당함.
				selected_BN_other[channel][version] = backend_node;
			}
		}
	}
}


void method_PA_AP(int power_limit) {
	set<pair<int, int>, less<pair<int, int>>> low_power_first; // 가장 사용 전력이 낮은 노드 선택

	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		int bn_type = (backend_node - 1) % BN_TYPE_NUM + 1;
		low_power_first.insert(make_pair(get_power(backend_node, 0), backend_node));
	}

	bool flag = true;
	int backend_node = 0; //0이 백엔드 노드 인덱스가 되는게 아님
	set<pair<float, int>, greater<pair<float, int>>> high_pop_channel_first;

	bool is_full[BN_NUM + 1];
	for (int bk = 1; bk <= BN_NUM; bk++) {
		is_full[bk] = false;
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		do {
			if (!low_power_first.size()) {
				flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 채널은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
				break;
			}
			backend_node = (*low_power_first.begin()).second; // 가장 파워가 낮은 노드

			if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0)) {
				low_power_first.erase(*low_power_first.begin());
				if (!is_full[backend_node]) {
					is_full[backend_node] = true;
				}
				int full_node_cnt = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (is_full[bk])
						full_node_cnt++;
				}
				if (full_node_cnt >= BN_NUM) { // 모든 백엔드가 꽉 차있다면
					flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
					break;
				}
			}
		} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0));// 해당 노드가 꽉 찼는지 아닌지 확인

		if (flag) {
			float power_sum = 0;
			for (int bk = 1; bk <= BN_NUM; bk++) {
				if (backend_node == bk)
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][1]));
				else
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
			}//전력이 꽉 찼는지 아닌지 확인
			if (power_sum > power_limit) {
				flag = false; //이제 할당 중지한다.
				//printf("1. RR_AP() : 1번 다 못했는데 꽉 참\n");
				break;
			}

			low_power_first.erase(*low_power_first.begin()); //CPU 사용율에 따른 update를 하기 위해 삭제
			remained_GHz[backend_node] -= info_other->video_GHz[channel][1]; // 1번 버전을 가장 전력이 낮은 노드 먼저 할당함.
			low_power_first.insert(make_pair(get_power(backend_node, get_backend_max_GHz(backend_node) - remained_GHz[backend_node]), backend_node));
			//CPU 사용율에 따른 update 해줌.

			selected_BN_other[channel][1] = backend_node;
		}

		high_pop_channel_first.insert(make_pair(info_other->get_channel_popularity(channel), channel));
	}

	int high_channel_cnt = 0;
	while (high_pop_channel_first.size() && flag) { //그 다음 2번 버전~ 6번 버전은 pop 순으로 라운드 로빈으로 할당함.
		int channel = (*high_pop_channel_first.begin()).second; //가장 높은 pop인 채널
		high_pop_channel_first.erase(*high_pop_channel_first.begin()); // 맨 위의 값 삭제

		for (int version = 2; version <= info_other->version_num - 1; version++) {
			do {
				if (!low_power_first.size()) {
					flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 채널은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
					break;
				}
				backend_node = (*low_power_first.begin()).second; // 가장 파워가 낮은 노드
				if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0)) {
					low_power_first.erase(*low_power_first.begin());
					if (!is_full[backend_node]) {
						is_full[backend_node] = true;
					}
					int full_node_cnt = 0;
					for (int bk = 1; bk <= BN_NUM; bk++) {
						if (is_full[bk])
							full_node_cnt++;
					}
					if (full_node_cnt >= BN_NUM) { // 모든 백엔드가 꽉 차있다면
						flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
						//현재 version 이전거 ~ 2번까지 할당한 걸 삭제한다.
						for (int ver = (version - 1); ver >= 2; ver--) {
							remained_GHz[selected_BN_other[channel][ver]] += info_other->video_GHz[channel][ver];
							selected_BN_other[channel][ver] = 0;
						}
						break;
					}
				}
			} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0));// 해당 노드가 꽉 찼는지 아닌지 확인

			if (flag) {
				float power_sum = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (backend_node == bk)
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][version]));
					else
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
				}//전력이 꽉 찼는지 아닌지 확인
				if (power_sum > power_limit) {
					//현재 version 이전거 ~ 2번까지 할당한 걸 삭제한다.
					for (int ver = (version - 1); ver >= 2; ver--) {
						remained_GHz[selected_BN_other[channel][ver]] += info_other->video_GHz[channel][ver];
						selected_BN_other[channel][ver] = 0;
					}

					flag = false; //이제 할당 중지한다.
					break;
				}

				low_power_first.erase(*low_power_first.begin());//CPU 사용율에 따른 update를 하기 위해 삭제

				remained_GHz[backend_node] -= info_other->video_GHz[channel][version]; // 해당 버전을 가장 전력이 낮은 노드 먼저 할당함.
				low_power_first.insert(make_pair(get_power(backend_node, get_backend_max_GHz(backend_node) - remained_GHz[backend_node]), backend_node));//CPU 사용율에 따른 update 해줌.
				selected_BN_other[channel][version] = backend_node;
			}
		}
	}
}

void method_PA_HPF(int power_limit) {
	set<pair<int, int>, less<pair<int, int>>> low_power_first; // 가장 사용 전력이 낮은 노드 선택

	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		int bn_type = (backend_node - 1) % BN_TYPE_NUM + 1;
		low_power_first.insert(make_pair(get_power(backend_node, 0), backend_node));
	}

	bool flag = true;
	int backend_node = 0; //0이 백엔드 노드 인덱스가 되는게 아님
	int full_node_cnt = 0;

	bool is_full[BN_NUM + 1];
	for (int bk = 1; bk <= BN_NUM; bk++) {
		is_full[bk] = false;
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		int full_node_cnt = 0;
		do {
			if (!low_power_first.size()) {
				flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
				break;
			}
			backend_node = (*low_power_first.begin()).second; // 가장 파워가 낮은 노드

			if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0)) {
				low_power_first.erase(*low_power_first.begin());
				if (!is_full[backend_node]) {
					is_full[backend_node] = true;
				}
				int full_node_cnt = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (is_full[bk])
						full_node_cnt++;
				}
				if (full_node_cnt >= BN_NUM) { // 모든 백엔드가 꽉 차있다면
					flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
					break;
				}
			}
		} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0));// 해당 노드가 꽉 찼는지 아닌지 확인

		if (flag) {
			float power_sum = 0;
			for (int bk = 1; bk <= BN_NUM; bk++) {
				if (backend_node == bk)
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][1]));
				else
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
			}//전력이 꽉 찼는지 아닌지 확인
			if (power_sum > power_limit) {
				flag = false; //이제 할당 중지한다.
				//printf("1. RR_AP() : 1번 다 못했는데 꽉 참\n");
				break;
			}

			low_power_first.erase(*low_power_first.begin()); //CPU 사용율에 따른 update를 하기 위해 삭제
			remained_GHz[backend_node] -= info_other->video_GHz[channel][1]; // 1번 버전을 가장 전력이 낮은 노드 먼저 할당함.
			low_power_first.insert(make_pair(get_power(backend_node, get_backend_max_GHz(backend_node) - remained_GHz[backend_node]), backend_node));
			//CPU 사용율에 따른 update 해줌.

			selected_BN_other[channel][1] = backend_node;
		}
	}

	set<pair<float, int>, greater<pair<float, int>> > high_pop_version_first;  //가장 평균 pop이 높은 버전을 찾기 위함
	float* version_pop_average = (float*)malloc(sizeof(float) * (info_other->version_num)); // 버전 1은 이미 트랜스코딩했고 마지막버전은 원본이라 트랜스코딩 안함
	for (int version = 2; version <= info_other->version_num - 1; version++) {
		version_pop_average[version] = 0;
		for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
			version_pop_average[version] += info_other->popularity[channel][version];
		}
		version_pop_average[version] /= CHANNEL_NUM;
	}
	for (int version = 2; version <= info_other->version_num - 1; version++) {
		high_pop_version_first.insert(make_pair(version_pop_average[version], version));
	}
	//여기까지 세팅

	while (high_pop_version_first.size() && flag) {
		int version = (*high_pop_version_first.begin()).second; // 가장 높은 pop인 버전
		high_pop_version_first.erase(*high_pop_version_first.begin()); // 맨 위의 값 삭제
		for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
			do {
				if (!low_power_first.size()) {
					flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
					break;
				}
				backend_node = (*low_power_first.begin()).second; // 가장 파워가 낮은 노드
				if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0)) {
					low_power_first.erase(*low_power_first.begin());
					if (!is_full[backend_node]) {
						is_full[backend_node] = true;
					}
					int full_node_cnt = 0;
					for (int bk = 1; bk <= BN_NUM; bk++) {
						if (is_full[bk])
							full_node_cnt++;
					}
					if (full_node_cnt >= BN_NUM) { // 모든 백엔드가 꽉 차있다면
						flag = false; //이제 할당 중지한다. 이것보다 더 낮은 인기도의 버전은 만약 꽉 안찬다 쳐도, 그들을 고려할 필요는 없다.
						break;
					}
				}
			} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0));// 해당 노드가 꽉 찼는지 아닌지 확인

			if (flag) {
				float power_sum = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (backend_node == bk)
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][version]));
					else
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
				}//전력이 꽉 찼는지 아닌지 확인
				if (power_sum > power_limit) {
					flag = false; //이제 할당 중지한다.
					break;
				}

				low_power_first.erase(*low_power_first.begin());//CPU 사용율에 따른 update를 하기 위해 삭제

				remained_GHz[backend_node] -= info_other->video_GHz[channel][version]; // 해당 버전을 가장 전력이 낮은 노드 먼저 할당함.
				low_power_first.insert(make_pair(get_power(backend_node, get_backend_max_GHz(backend_node) - remained_GHz[backend_node]), backend_node));
				//CPU 사용율에 따른 update 해줌.
				selected_BN_other[channel][version] = backend_node;
			}
		}
	}
}
