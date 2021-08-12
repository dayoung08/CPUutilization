#include "head.h"

//�� ��Ŵ���� ¥�� ���� �ڵ��̴�.
short selected_set_other[CHANNEL_NUM + 1]; // �� ä�ο��� ����ϴ� ��Ʈ����Ʈ set
short** selected_BN_other;//[CHANNEL_NUM + 1][VERSION_NUM]; // �� ä�ο��� ����ϴ� ��Ʈ����Ʈ set�� ���ϴ� �� ������ � BN���� ���õǾ��°�.
//�������� ������ Ʈ�����ڵ� ���ؼ� �迭 ũ�Ⱑ ������.

float remained_GHz[BN_NUM + 1];//��� ����� ���� Hz ����� ����
bitrate_version_set_info* info_other;

void other_method(int method_index, int power_limit, bitrate_version_set_info* _info) {
	info_other = _info;

	selected_BN_other = (short**)malloc(sizeof(short*) * (CHANNEL_NUM + 1));
	for (int row = 1; row <= CHANNEL_NUM; row++) {
		selected_BN_other[row] = (short*)malloc(sizeof(short) * (info_other->version_num));  // �������� ������ Ʈ�����ڵ� ���ϴϱ�
		for (int col = 1; col < info_other->version_num; col++) {  // �������� ������ Ʈ�����ڵ� ���ϴϱ�
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
	float pwq_sum_range[10]; // 90~100%, 80~90% ... 0~10%������ Average PWQ�� ��.
	float video_quality_sum_range[10]; // 90~100%, 80~90% ... 0~10%������ Average video_quality�� ��.
	for (int per_index = 0; per_index < 10; per_index++) {
		video_quality_sum_range[per_index] = 0;
		pwq_sum_range[per_index] = 0;
	}

	int* number_of_transcoding = (int*)malloc(sizeof(int) * (info_other->version_num));
	for (int version = 1; version <= info_other->version_num - 1; version++) {
		number_of_transcoding[version] = 0;
	}
	// �� ��Ʈ����Ʈ ���� �� ���� transcoding�� ���� ��� ��. �� ������ range�� [0, CHANNEL_NUM]

	//printf("[ä��-��Ʈ]\n");
	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		/*if (info_other->pwq_set[channel][selected_set_other[channel]] < 0) {
			cout << "�����";
		}*/
		pwq_sum += info_other->pwq_set[channel][selected_set_other[channel]];
		video_quality_sum += info_other->video_quality_set[channel][selected_set_other[channel]];
		for (int version = 1; version <= info_other->version_num - 1; version++) {
			if (((selected_set_other[channel] - 1) & info_other->version_set_num >> ((info_other->version_num - 2) - (version - 1)) || (version == 1))) {
				number_of_transcoding[version]++;
			}
		}
		video_quality_sum_range[(channel - 1) / (CHANNEL_NUM / 10)] += info_other->video_quality_set[channel][selected_set_other[channel]]; // 90~100%, 80~90% ... 0~10%������ Average video_quality�� ��.
		pwq_sum_range[(channel - 1) / (CHANNEL_NUM / 10)] += info_other->pwq_set[channel][selected_set_other[channel]]; // 90~100%, 80~90% ... 0~10%������ Average video_quality�� ��.
	}
	printf("\n��ü pwq�� �� : %lf", pwq_sum);
	printf("\n��ü video_quality�� ��� : %lf\n\n", video_quality_sum / CHANNEL_NUM / info_other->version_num);
	/*for (int per_index = 0; per_index < 10; per_index++) {
		printf("%d%~%d%% pwq�� ��/video_quality�� ��� : %lf, %lf\n", (per_index * 10), (per_index + 1) * 10, pwq_sum_range[per_index], video_quality_sum_range[per_index] / (CHANNEL_NUM / 10) / info_other->version_num);
	}*/

	int power_sum = 0;
	float util_sum = 0;
	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		//printf("[�鿣�� ��� %d]\n", backend_node);
		//printf("���� GHz, �ִ� GHz : %lf / %d\n", remained_GHz[backend_node], get_backend_max_GHz(backend_node));
		float util = get_backend_max_GHz(backend_node) - remained_GHz[backend_node];
		power_sum += get_power(backend_node, util); // ��ü���� ���� �� ���� ����� GHz
		util_sum += util;
	}
	printf("\n��� ���� : %d W\n", power_sum);
	printf("��� GHz : %lf GHz\n\n", util_sum);

	for (int version = info_other->version_num - 1; version >= 2; version--) {
		printf("���� %d : %d\n", version, number_of_transcoding[version]);
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
	int backend_node = 0; //0���� �����ϴ°��� 0�� �鿣�� ��� �ε����� �Ǵ°� �ƴ�
	set<pair<float, int>, greater<pair<float, int>>> high_pop_channel_first;

	bool is_full[BN_NUM + 1];
	for (int bk = 1; bk <= BN_NUM; bk++) {
		is_full[bk] = false;
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		do {
			backend_node++; //RR�� backend node �Ҵ��ϱ� ���� ++
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
				if (full_node_cnt >= BN_NUM) { // ��� �鿣�尡 �� ���ִٸ�
					flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
					break;
				}
			}
		} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0));// �ش� ��尡 �� á���� �ƴ��� Ȯ��

		if (flag) {
			float power_sum = 0;
			for (int bk = 1; bk <= BN_NUM; bk++) {
				if (backend_node == bk)
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][1]));
				else
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
			}//������ �� á���� �ƴ��� Ȯ��
			if (power_sum > power_limit) {
				flag = false; //���� �Ҵ� �����Ѵ�.
				//printf("1. RR_AP() : 1�� �� ���ߴµ� �� ��\n");
				break;
			}

			remained_GHz[backend_node] -= info_other->video_GHz[channel][1]; // 1�� ������ ���� �κ� ���·� �Ҵ���.
			selected_BN_other[channel][1] = backend_node;
		}

		high_pop_channel_first.insert(make_pair(info_other->get_channel_popularity(channel), channel));
	}

	int high_channel_cnt = 0;
	while (high_pop_channel_first.size() && flag) { //�� ���� 2�� ����~ 6�� ������ pop ������ ���� �κ����� �Ҵ���.
		int channel = (*high_pop_channel_first.begin()).second; //���� ���� pop�� ä��
		high_pop_channel_first.erase(*high_pop_channel_first.begin()); // �� ���� �� ����

		for (int version = 2; version <= info_other->version_num - 1; version++) {
			do {
				backend_node++; //RR�� backend node �Ҵ��ϱ� ���� ++
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
					if (full_node_cnt >= BN_NUM) { // ��� �鿣�尡 �� ���ִٸ�
						flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
						//���� version ������ ~ 2������ �Ҵ��� �� �����Ѵ�.
						for (int ver = (version - 1); ver >= 2; ver--) {
							remained_GHz[selected_BN_other[channel][ver]] += info_other->video_GHz[channel][ver];
							selected_BN_other[channel][ver] = 0;
						}
						break;
					}
				}
			} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0));// �ش� ��尡 �� á���� �ƴ��� Ȯ��

			if (flag) {
				float power_sum = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (backend_node == bk)
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][version]));
					else
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
				}//������ �� á���� �ƴ��� Ȯ��
				if (power_sum > power_limit) {
					//���� version ������ ~ 2������ �Ҵ��� �� �����Ѵ�.
					for (int ver = (version - 1); ver >= 2; ver--) {
						remained_GHz[selected_BN_other[channel][ver]] += info_other->video_GHz[channel][ver];
						selected_BN_other[channel][ver] = 0;
					}

					flag = false; //���� �Ҵ� �����Ѵ�.
					break;
				}

				remained_GHz[backend_node] -= info_other->video_GHz[channel][version]; // ������ ������ ���� �κ� ���·� �Ҵ���.
				selected_BN_other[channel][version] = backend_node;
			}
		}
	}
}

void method_RR_HPF(int power_limit) {
	bool flag = true;
	int backend_node = 0; //0���� �����ϴ°��� 0�� �鿣�� ��� �ε����� �Ǵ°� �ƴ�

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		bool is_full[BN_NUM + 1];
		for (int bk = 1; bk <= BN_NUM; bk++) {
			is_full[bk] = false;
		}
		do {
			backend_node++; //RR�� backend node �Ҵ��ϱ� ���� ++
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
				if (full_node_cnt >= BN_NUM) { // ��� �鿣�尡 �� ���ִٸ�
					flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
					break;
				}
			}
		} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0));// �ش� ��尡 �� á���� �ƴ��� Ȯ��

		if (flag) {
			float power_sum = 0;
			for (int bk = 1; bk <= BN_NUM; bk++) {
				if (backend_node == bk)
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][1]));
				else
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
			}//������ �� á���� �ƴ��� Ȯ��
			if (power_sum > power_limit) {
				flag = false; //���� �Ҵ� �����Ѵ�.
				//printf("1. RR_AP() : 1�� �� ���ߴµ� �� ��\n");
				break;
			}

			remained_GHz[backend_node] -= info_other->video_GHz[channel][1]; // 1�� ������ ���� �κ� ���·� �Ҵ���.
			selected_BN_other[channel][1] = backend_node;
		}
	}

	set<pair<float, int>, greater<pair<float, int>> > high_pop_version_first;  //���� ��� pop�� ���� ������ ã�� ����
	float* version_pop_average = (float*)malloc(sizeof(float) * (info_other->version_num)); // ���� 1�� �̹� Ʈ�����ڵ��߰� ������������ �����̶� Ʈ�����ڵ� ����
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
	//������� ����

	while (high_pop_version_first.size() && flag) {
		int version = (*high_pop_version_first.begin()).second; // ���� ���� pop�� ����
		high_pop_version_first.erase(*high_pop_version_first.begin()); // �� ���� �� ����
		for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
			bool is_full[BN_NUM + 1];
			for (int bk = 1; bk <= BN_NUM; bk++) {
				is_full[bk] = false;
			}
			do {
				backend_node++; //RR�� backend node �Ҵ��ϱ� ���� ++
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
					if (full_node_cnt >= BN_NUM) { // ��� �鿣�尡 �� ���ִٸ�
						flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
						break;
					}
				}
			} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version]) < 0);// �ش� ��尡 �� á���� �ƴ��� Ȯ��

			if (flag) {
				float power_sum = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (backend_node == bk)
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][version]));
					else
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
				}//������ �� á���� �ƴ��� Ȯ��
				if (power_sum > power_limit) {
					flag = false; //���� �Ҵ� �����Ѵ�.
					break;
				}

				remained_GHz[backend_node] -= info_other->video_GHz[channel][version]; // ������ ������ ���� �κ� ���·� �Ҵ���.
				selected_BN_other[channel][version] = backend_node;
			}
		}
	}
}


void method_RA_AP(int power_limit) {
	bool flag = true;
	int backend_node = 0; //0���� �����ϴ°��� 0�� �鿣�� ��� �ε����� �Ǵ°� �ƴ�
	set<pair<float, int>, greater<pair<float, int>>> high_pop_channel_first;

	bool is_full[BN_NUM + 1];
	for (int bk = 1; bk <= BN_NUM; bk++) {
		is_full[bk] = false;
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		do {
			backend_node = rand() % BN_NUM + 1; //�������� �鿣�� ��� �Ҵ�

			if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0)) {
				if (!is_full[backend_node]) {
					is_full[backend_node] = true;
				}
				int full_node_cnt = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (is_full[bk])
						full_node_cnt++;
				}
				if (full_node_cnt >= BN_NUM) { // ��� �鿣�尡 �� ���ִٸ�
					flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
					break;
				}
			}
		} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0));// �ش� ��尡 �� á���� �ƴ��� Ȯ��

		if (flag) {
			float power_sum = 0;
			for (int bk = 1; bk <= BN_NUM; bk++) {
				if (backend_node == bk)
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][1]));
				else
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
			}//������ �� á���� �ƴ��� Ȯ��
			if (power_sum > power_limit) {
				flag = false; //���� �Ҵ� �����Ѵ�.
				//printf("1. RR_AP() : 1�� �� ���ߴµ� �� ��\n");
				break;
			}

			remained_GHz[backend_node] -= info_other->video_GHz[channel][1]; // 1�� ������ ���� �κ� ���·� �Ҵ���.
			selected_BN_other[channel][1] = backend_node;
		}

		high_pop_channel_first.insert(make_pair(info_other->get_channel_popularity(channel), channel));
	}

	int high_channel_cnt = 0;
	while (high_pop_channel_first.size() && flag) { //�� ���� 2�� ����~ 6�� ������ pop ������ ���� �κ����� �Ҵ���.
		int channel = (*high_pop_channel_first.begin()).second; //���� ���� pop�� ä��
		high_pop_channel_first.erase(*high_pop_channel_first.begin()); // �� ���� �� ����

		for (int version = 2; version <= info_other->version_num - 1; version++) {
			do {
				backend_node = rand() % BN_NUM + 1; //�������� �鿣�� ��� �Ҵ�

				if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0)) {
					if (!is_full[backend_node]) {
						is_full[backend_node] = true;
					}
					int full_node_cnt = 0;
					for (int bk = 1; bk <= BN_NUM; bk++) {
						if (is_full[bk])
							full_node_cnt++;
					}
					if (full_node_cnt >= BN_NUM) { // ��� �鿣�尡 �� ���ִٸ�
						flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
						//���� version ������ ~ 2������ �Ҵ��� �� �����Ѵ�.
						for (int ver = (version - 1); ver >= 2; ver--) {
							remained_GHz[selected_BN_other[channel][ver]] += info_other->video_GHz[channel][ver];
							selected_BN_other[channel][ver] = 0;
						}
						break;
					}
				}
			} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0));// �ش� ��尡 �� á���� �ƴ��� Ȯ��

			if (flag) {
				float power_sum = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (backend_node == bk)
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][version]));
					else
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
				}//������ �� á���� �ƴ��� Ȯ��
				if (power_sum > power_limit) {
					//���� version ������ ~ 2������ �Ҵ��� �� �����Ѵ�.
					for (int ver = (version - 1); ver >= 2; ver--) {
						remained_GHz[selected_BN_other[channel][ver]] += info_other->video_GHz[channel][ver];
						selected_BN_other[channel][ver] = 0;
					}

					flag = false; //���� �Ҵ� �����Ѵ�.
					break;
				}

				remained_GHz[backend_node] -= info_other->video_GHz[channel][version]; // ������ ������ ���� �κ� ���·� �Ҵ���.
				selected_BN_other[channel][version] = backend_node;
			}
		}
	}
}

void method_RA_HPF(int power_limit) {
	bool flag = true;
	int backend_node = 0; //0���� �����ϴ°��� 0�� �鿣�� ��� �ε����� �Ǵ°� �ƴ�

	bool is_full[BN_NUM + 1];
	for (int bk = 1; bk <= BN_NUM; bk++) {
		is_full[bk] = false;
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		int full_node_cnt = 0;
		do {
			backend_node = rand() % BN_NUM + 1; //�������� �鿣�� ��� �Ҵ�

			if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0)) {
				if (!is_full[backend_node]) {
					is_full[backend_node] = true;
				}
				int full_node_cnt = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (is_full[bk])
						full_node_cnt++;
				}
				if (full_node_cnt >= BN_NUM) { // ��� �鿣�尡 �� ���ִٸ�
					flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
					//���� version ������ ~ 2������ �Ҵ��� �� �����Ѵ�.
					break;
				}
			}
		} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0));// �ش� ��尡 �� á���� �ƴ��� Ȯ��

		if (flag) {
			float power_sum = 0;
			for (int bk = 1; bk <= BN_NUM; bk++) {
				if (backend_node == bk)
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][1]));
				else
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
			}//������ �� á���� �ƴ��� Ȯ��
			if (power_sum > power_limit) {
				flag = false; //���� �Ҵ� �����Ѵ�.
				//printf("1. RR_AP() : 1�� �� ���ߴµ� �� ��\n");
				break;
			}

			remained_GHz[backend_node] -= info_other->video_GHz[channel][1]; // 1�� ������ ���� �κ� ���·� �Ҵ���.
			selected_BN_other[channel][1] = backend_node;
		}
	}

	set<pair<float, int>, greater<pair<float, int>> > high_pop_version_first;  //���� ��� pop�� ���� ������ ã�� ����
	float* version_pop_average = (float*)malloc(sizeof(float) * (info_other->version_num)); // ���� 1�� �̹� Ʈ�����ڵ��߰� ������������ �����̶� Ʈ�����ڵ� ����
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
	//������� ����

	while (high_pop_version_first.size() && flag) {
		int version = (*high_pop_version_first.begin()).second; // ���� ���� pop�� ����
		high_pop_version_first.erase(*high_pop_version_first.begin()); // �� ���� �� ����
		for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
			do {
				backend_node = rand() % BN_NUM + 1; //�������� �鿣�� ��� �Ҵ�

				if (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0)) {
					if (!is_full[backend_node]) {
						is_full[backend_node] = true;
					}
					int full_node_cnt = 0;
					for (int bk = 1; bk <= BN_NUM; bk++) {
						if (is_full[bk])
							full_node_cnt++;
					}
					if (full_node_cnt >= BN_NUM) { // ��� �鿣�尡 �� ���ִٸ�
						flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
						break;
					}
				}
			} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0));// �ش� ��尡 �� á���� �ƴ��� Ȯ��

			if (flag) {
				float power_sum = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (backend_node == bk)
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][version]));
					else
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
				}//������ �� á���� �ƴ��� Ȯ��
				if (power_sum > power_limit) {
					flag = false; //���� �Ҵ� �����Ѵ�.
					break;
				}

				remained_GHz[backend_node] -= info_other->video_GHz[channel][version]; // ������ ������ ���� �κ� ���·� �Ҵ���.
				selected_BN_other[channel][version] = backend_node;
			}
		}
	}
}


void method_PA_AP(int power_limit) {
	set<pair<int, int>, less<pair<int, int>>> low_power_first; // ���� ��� ������ ���� ��� ����

	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		int bn_type = (backend_node - 1) % BN_TYPE_NUM + 1;
		low_power_first.insert(make_pair(get_power(backend_node, 0), backend_node));
	}

	bool flag = true;
	int backend_node = 0; //0�� �鿣�� ��� �ε����� �Ǵ°� �ƴ�
	set<pair<float, int>, greater<pair<float, int>>> high_pop_channel_first;

	bool is_full[BN_NUM + 1];
	for (int bk = 1; bk <= BN_NUM; bk++) {
		is_full[bk] = false;
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		do {
			if (!low_power_first.size()) {
				flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ä���� ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
				break;
			}
			backend_node = (*low_power_first.begin()).second; // ���� �Ŀ��� ���� ���

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
				if (full_node_cnt >= BN_NUM) { // ��� �鿣�尡 �� ���ִٸ�
					flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
					break;
				}
			}
		} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0));// �ش� ��尡 �� á���� �ƴ��� Ȯ��

		if (flag) {
			float power_sum = 0;
			for (int bk = 1; bk <= BN_NUM; bk++) {
				if (backend_node == bk)
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][1]));
				else
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
			}//������ �� á���� �ƴ��� Ȯ��
			if (power_sum > power_limit) {
				flag = false; //���� �Ҵ� �����Ѵ�.
				//printf("1. RR_AP() : 1�� �� ���ߴµ� �� ��\n");
				break;
			}

			low_power_first.erase(*low_power_first.begin()); //CPU ������� ���� update�� �ϱ� ���� ����
			remained_GHz[backend_node] -= info_other->video_GHz[channel][1]; // 1�� ������ ���� ������ ���� ��� ���� �Ҵ���.
			low_power_first.insert(make_pair(get_power(backend_node, get_backend_max_GHz(backend_node) - remained_GHz[backend_node]), backend_node));
			//CPU ������� ���� update ����.

			selected_BN_other[channel][1] = backend_node;
		}

		high_pop_channel_first.insert(make_pair(info_other->get_channel_popularity(channel), channel));
	}

	int high_channel_cnt = 0;
	while (high_pop_channel_first.size() && flag) { //�� ���� 2�� ����~ 6�� ������ pop ������ ���� �κ����� �Ҵ���.
		int channel = (*high_pop_channel_first.begin()).second; //���� ���� pop�� ä��
		high_pop_channel_first.erase(*high_pop_channel_first.begin()); // �� ���� �� ����

		for (int version = 2; version <= info_other->version_num - 1; version++) {
			do {
				if (!low_power_first.size()) {
					flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ä���� ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
					break;
				}
				backend_node = (*low_power_first.begin()).second; // ���� �Ŀ��� ���� ���
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
					if (full_node_cnt >= BN_NUM) { // ��� �鿣�尡 �� ���ִٸ�
						flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
						//���� version ������ ~ 2������ �Ҵ��� �� �����Ѵ�.
						for (int ver = (version - 1); ver >= 2; ver--) {
							remained_GHz[selected_BN_other[channel][ver]] += info_other->video_GHz[channel][ver];
							selected_BN_other[channel][ver] = 0;
						}
						break;
					}
				}
			} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0));// �ش� ��尡 �� á���� �ƴ��� Ȯ��

			if (flag) {
				float power_sum = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (backend_node == bk)
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][version]));
					else
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
				}//������ �� á���� �ƴ��� Ȯ��
				if (power_sum > power_limit) {
					//���� version ������ ~ 2������ �Ҵ��� �� �����Ѵ�.
					for (int ver = (version - 1); ver >= 2; ver--) {
						remained_GHz[selected_BN_other[channel][ver]] += info_other->video_GHz[channel][ver];
						selected_BN_other[channel][ver] = 0;
					}

					flag = false; //���� �Ҵ� �����Ѵ�.
					break;
				}

				low_power_first.erase(*low_power_first.begin());//CPU ������� ���� update�� �ϱ� ���� ����

				remained_GHz[backend_node] -= info_other->video_GHz[channel][version]; // �ش� ������ ���� ������ ���� ��� ���� �Ҵ���.
				low_power_first.insert(make_pair(get_power(backend_node, get_backend_max_GHz(backend_node) - remained_GHz[backend_node]), backend_node));//CPU ������� ���� update ����.
				selected_BN_other[channel][version] = backend_node;
			}
		}
	}
}

void method_PA_HPF(int power_limit) {
	set<pair<int, int>, less<pair<int, int>>> low_power_first; // ���� ��� ������ ���� ��� ����

	for (int backend_node = 1; backend_node <= BN_NUM; backend_node++) {
		int bn_type = (backend_node - 1) % BN_TYPE_NUM + 1;
		low_power_first.insert(make_pair(get_power(backend_node, 0), backend_node));
	}

	bool flag = true;
	int backend_node = 0; //0�� �鿣�� ��� �ε����� �Ǵ°� �ƴ�
	int full_node_cnt = 0;

	bool is_full[BN_NUM + 1];
	for (int bk = 1; bk <= BN_NUM; bk++) {
		is_full[bk] = false;
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		int full_node_cnt = 0;
		do {
			if (!low_power_first.size()) {
				flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
				break;
			}
			backend_node = (*low_power_first.begin()).second; // ���� �Ŀ��� ���� ���

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
				if (full_node_cnt >= BN_NUM) { // ��� �鿣�尡 �� ���ִٸ�
					flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
					break;
				}
			}
		} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][1] < 0));// �ش� ��尡 �� á���� �ƴ��� Ȯ��

		if (flag) {
			float power_sum = 0;
			for (int bk = 1; bk <= BN_NUM; bk++) {
				if (backend_node == bk)
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][1]));
				else
					power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
			}//������ �� á���� �ƴ��� Ȯ��
			if (power_sum > power_limit) {
				flag = false; //���� �Ҵ� �����Ѵ�.
				//printf("1. RR_AP() : 1�� �� ���ߴµ� �� ��\n");
				break;
			}

			low_power_first.erase(*low_power_first.begin()); //CPU ������� ���� update�� �ϱ� ���� ����
			remained_GHz[backend_node] -= info_other->video_GHz[channel][1]; // 1�� ������ ���� ������ ���� ��� ���� �Ҵ���.
			low_power_first.insert(make_pair(get_power(backend_node, get_backend_max_GHz(backend_node) - remained_GHz[backend_node]), backend_node));
			//CPU ������� ���� update ����.

			selected_BN_other[channel][1] = backend_node;
		}
	}

	set<pair<float, int>, greater<pair<float, int>> > high_pop_version_first;  //���� ��� pop�� ���� ������ ã�� ����
	float* version_pop_average = (float*)malloc(sizeof(float) * (info_other->version_num)); // ���� 1�� �̹� Ʈ�����ڵ��߰� ������������ �����̶� Ʈ�����ڵ� ����
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
	//������� ����

	while (high_pop_version_first.size() && flag) {
		int version = (*high_pop_version_first.begin()).second; // ���� ���� pop�� ����
		high_pop_version_first.erase(*high_pop_version_first.begin()); // �� ���� �� ����
		for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
			do {
				if (!low_power_first.size()) {
					flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
					break;
				}
				backend_node = (*low_power_first.begin()).second; // ���� �Ŀ��� ���� ���
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
					if (full_node_cnt >= BN_NUM) { // ��� �鿣�尡 �� ���ִٸ�
						flag = false; //���� �Ҵ� �����Ѵ�. �̰ͺ��� �� ���� �α⵵�� ������ ���� �� ������ �ĵ�, �׵��� ����� �ʿ�� ����.
						break;
					}
				}
			} while (is_full[backend_node] || (remained_GHz[backend_node] - info_other->video_GHz[channel][version] < 0));// �ش� ��尡 �� á���� �ƴ��� Ȯ��

			if (flag) {
				float power_sum = 0;
				for (int bk = 1; bk <= BN_NUM; bk++) {
					if (backend_node == bk)
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk] - info_other->video_GHz[channel][version]));
					else
						power_sum += get_power(bk, get_backend_max_GHz(bk) - (remained_GHz[bk]));
				}//������ �� á���� �ƴ��� Ȯ��
				if (power_sum > power_limit) {
					flag = false; //���� �Ҵ� �����Ѵ�.
					break;
				}

				low_power_first.erase(*low_power_first.begin());//CPU ������� ���� update�� �ϱ� ���� ����

				remained_GHz[backend_node] -= info_other->video_GHz[channel][version]; // �ش� ������ ���� ������ ���� ��� ���� �Ҵ���.
				low_power_first.insert(make_pair(get_power(backend_node, get_backend_max_GHz(backend_node) - remained_GHz[backend_node]), backend_node));
				//CPU ������� ���� update ����.
				selected_BN_other[channel][version] = backend_node;
			}
		}
	}
}
