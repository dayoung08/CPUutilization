#include "head.h"

//이 코드에서 processing capacity는 int로 되어있음.
int backend_max_GHz[BN_TYPE_NUM + 1] = { 0, 288, 256, 256, 302,   156, 864, 184, 1254, 324, 313 };
//ASUSTeK Computer Inc. RS700A-E9-RS4V2 http://www.spec.org/power_ssj2008/results/res2020q2/power_ssj2008-20200313-01020.html
//Dell Inc. PowerEdge R7525 http://www.spec.org/power_ssj2008/results/res2020q2/power_ssj2008-20200324-01021.html
//Dell Inc. PowerEdge R6525 http://www.spec.org/power_ssj2008/results/res2020q1/power_ssj2008-20200310-01018.html
//Hewlett Packard Enterprise ProLiant DL580 Gen10 http://www.spec.org/power_ssj2008/results/res2019q2/power_ssj2008-20190311-00883.html
//추가 2020-21
//Hewlett Packard Enterprise ProLiant DL325 Gen10 https://www.spec.org/power_ssj2008/results/res2021q1/power_ssj2008-20210221-01066.html
//ASUSTeK Computer Inc. RS620SA-E10-RS12 https://www.spec.org/power_ssj2008/results/res2020q4/power_ssj2008-20200918-01046.html
//Fujitsu PRIMERGY RX2530 M6 https://www.spec.org/power_ssj2008/results/res2021q2/power_ssj2008-20210407-01092.html
//Hewlett Packard Enterprise Apollo XL225n Gen10 Plus https://www.spec.org/power_ssj2008/results/res2021q1/power_ssj2008-20210223-01073.html
//Fujitsu PRIMERGY RX4770 M6 https://www.spec.org/power_ssj2008/results/res2020q4/power_ssj2008-20201006-01049.html
//Lenovo Global Technology ThinkSystem SR665 https://www.spec.org/power_ssj2008/results/res2021q2/power_ssj2008-20210408-01094.html

/*float video_quality[CHANNEL_NUM + 1][info->version_num + 1];
float popularity[CHANNEL_NUM + 1][info->version_num + 1]; // [채널][0] 여기는 전체 버전 version의 pop합.
float video_GHz[CHANNEL_NUM + 1][info->version_num + 1];
float pwq[CHANNEL_NUM + 1][info->version_num + 1]; // weighted video quality라고 쓰여있음. 하도 평소에 video pwq이라 불러서 코딩때도 이렇게 함.

float video_quality_set[CHANNEL_NUM + 1][VERSION_SET_NUM + 1];
float pwq_set[CHANNEL_NUM + 1][VERSION_SET_NUM + 1];

float sum_of_version_set_GHz[CHANNEL_NUM + 1][VERSION_SET_NUM + 1];*/

bitrate_version_set_info::bitrate_version_set_info(int _index) {
	index = _index;
	if (index == 0) { //Zecoder
		version_num = 7;
	}
	else if (index == 1) { // Youtube
		version_num = 11;
	}
	else if (index == 2) { // Neflix
		version_num = 10;
	}
	else if (index == 4) { // 논문 https://doi.org/10.1145/3123266.3123426
		version_num = 9;
	}
	else if (index == 3) { // IBM Watson Media
		version_num = 7;
	}

	version_set_num = pow(2, (version_num - 2));
	resolution = (int*)malloc(sizeof(int) * (version_num + 1));
	bitrate = (int*)malloc(sizeof(int) * (version_num + 1));

	if (index == 0) { //Zecoder
		resolution[7] = 19201080; // 1920x1080
		resolution[6] = 1280720; // 1280x720
		resolution[5] = 640360; // 640x360
		resolution[4] = 640360; // 640x360
		resolution[3] = 400224; // 400x224
		resolution[2] = 400224; // 400x224
		resolution[1] = 400224; // 400x224

		bitrate[7] = 2500; //kbps
		bitrate[6] = 2000;
		bitrate[5] = 1500;
		bitrate[4] = 1000;
		bitrate[3] = 600;
		bitrate[2] = 400;
		bitrate[1] = 200;
	}
	else if (index == 1) { // Youtube https://support.google.com/youtube/answer/2853702?hl=ko 기준. 비트레이트는 mean 사용.
		resolution[11] = 38402160; // 3840x2160
		resolution[10] = 38402160; // 3840x2160
		resolution[9] = 25601440; // 2560x1440
		resolution[8] = 25601440; // 2560x1440
		resolution[7] = 19201080; // 1920x1080
		resolution[6] = 19201080; // 1920x1080
		resolution[5] = 1280720; // 1280x720
		resolution[4] = 1280720; // 1280x720
		resolution[3] = 854480; //  854x480
		resolution[2] = 640360; //  640x360
		resolution[1] = 426240; //  426x240

		bitrate[11] = 35000;
		bitrate[10] = 23500;
		bitrate[9] = 13500;
		bitrate[8] = 9500;
		bitrate[7] = 6750;
		bitrate[6] = 4500;
		bitrate[5] = 4125;
		bitrate[4] = 2750;
		bitrate[3] = 1250;
		bitrate[2] = 700;
		bitrate[1] = 500;
	}
	else if (index == 2) { // Netflix https://netflixtechblog.com/per-title-encode-optimization-7e99442b62a2
		resolution[10] = 19201080; // 1920x1080
		resolution[9] = 19201080; // 1920x1080
		resolution[8] = 1280720; // 1280x720
		resolution[7] = 1280720; // 1280x720
		resolution[6] = 720480; // 720x480
		resolution[5] = 640480; // 640x480
		resolution[4] = 512384; // 512x384
		resolution[3] = 512384; // 512x384
		resolution[2] = 384288; // 384x288
		resolution[1] = 320240; // 320x240

		bitrate[10] = 5800;
		bitrate[9] = 4300;
		bitrate[8] = 3000;
		bitrate[7] = 2350;
		bitrate[6] = 1750;
		bitrate[5] = 1050;
		bitrate[4] = 750;
		bitrate[3] = 560;
		bitrate[2] = 375;
		bitrate[1] = 235;
	}
	else if (index == 4) {  // 논문 https://doi.org/10.1145/3123266.3123426
		resolution[9] = 19201080; // 1920x1080
		resolution[8] = 19201080; // 1920x1080
		resolution[7] = 19201080; // 1920x1080
		resolution[6] = 19201080; // 1920x1080
		resolution[5] = 1280720; // 1280x720
		resolution[4] = 1280720; // 1280x720
		resolution[3] = 640360; //  640x360
		resolution[2] = 640360; //  640x360
		resolution[1] = 480270; // 480x270

		bitrate[9] = 10000;
		bitrate[8] = 6000;
		bitrate[7] = 4000;
		bitrate[6] = 3000;
		bitrate[5] = 2400;
		bitrate[4] = 1500;
		bitrate[3] = 807;
		bitrate[2] = 505;
		bitrate[1] = 253;
	}
	else if (index == 3) {// IBM Watson Media
		resolution[7] = 38402160; // 3840x2160
		resolution[6] = 19201080; // 1920x1080
		resolution[5] = 1280720; // 1280x720
		resolution[4] = 960540; // 960x540
		resolution[3] = 854480; //  854x480
		resolution[2] = 640360; //  640x360
		resolution[1] = 480270; // 480x270

		bitrate[7] = 11000;
		bitrate[6] = 6000;
		bitrate[5] = 2750;
		bitrate[4] = 1350;
		bitrate[3] = 1350;
		bitrate[2] = 1000;
		bitrate[1] = 400;
	}

	video_quality = (float**)malloc(sizeof(float*) * (CHANNEL_NUM + 1));
	popularity = (float**)malloc(sizeof(float*) * (CHANNEL_NUM + 1)); //popularity[channel][0]은 그 채널의 pop이다.

	video_GHz = (float**)malloc(sizeof(float*) * (CHANNEL_NUM + 1));
	pwq = (float**)malloc(sizeof(float*) * (CHANNEL_NUM + 1));

	video_quality_set = (float**)malloc(sizeof(float*) * (CHANNEL_NUM + 1));
	pwq_set = (float**)malloc(sizeof(float*) * (CHANNEL_NUM + 1));
	sum_of_version_set_GHz = (float**)malloc(sizeof(float*) * (CHANNEL_NUM + 1));

	for (int row = 1; row <= CHANNEL_NUM; row++) {
		video_quality[row] = (float*)malloc(sizeof(float) * (version_num + 1));
		popularity[row] = (float*)malloc(sizeof(float) * (version_num + 1));
		video_GHz[row] = (float*)malloc(sizeof(float) * (version_num + 1));
		pwq[row] = (float*)malloc(sizeof(float) * (version_num + 1));

		video_quality_set[row] = (float*)malloc(sizeof(float) * (version_set_num + 1));
		pwq_set[row] = (float*)malloc(sizeof(float) * (version_set_num + 1));
		sum_of_version_set_GHz[row] = (float*)malloc(sizeof(float) * (version_set_num + 1));

		for (int col = 1; col <= version_num; col++) {
			video_quality[row][col] = 0;
			popularity[row][col] = 0;
			video_GHz[row][col] = 0;
			pwq[row][col] = 0;
		}
		for (int col = 1; col <= version_set_num; col++) {
			video_quality_set[row][col] = 0;
			pwq_set[row][col] = 0;
			sum_of_version_set_GHz[row][col] = 0;
		}
	}
	number_for_bit_opration = pow(2, version_num - 3);
	set_versions_number_for_bit_opration = version_num - 2;

}

float bitrate_version_set_info::get_channel_popularity(int channel) {
	return popularity[channel][0];
}

float* set_zipf_pop(int length, float alpha, float beta) {
	float* zipf = (float*)malloc(sizeof(float) * (length + 1));
	float* pop = (float*)malloc(sizeof(float) * (length + 1));
	float sum_caculatedValue = 0;
	float caculatedValue = 0;

	zipf[0] = 0;
	for (int i = 1; i < length + 1; i++) {
		caculatedValue = (float)beta / powl(i, alpha);
		sum_caculatedValue += caculatedValue;
		zipf[i] = caculatedValue;
	}
	float sum = 0;
	pop[0] = 0;
	for (int i = 1; i < length + 1; i++) {
		zipf[i] /= sum_caculatedValue;
		pop[i] = zipf[i];
	}
	return pop;
}

//https://en.wikipedia.org/wiki/Gamma_distribution
float* set_gamma_pop(float length, float k, float theta) {
	float sum = 0;
	float* gamma_pdf = (float*)malloc(sizeof(float) * (length + 1));
	for (int value = 1; value <= length; value++) {
		float result = (pow(value, (k - 1)) * std::pow(M_E, (-value / theta))) / (tgamma(k) * pow(theta, k));
		gamma_pdf[value] = result;
		sum += result;
	}

	for (int value = 1; value <= length; value++) {
		gamma_pdf[value] /= sum;
		//cout << gamma_pdf[value] << " ";
	}
	//채널 갯수 줄이기가 힘드니까 노드 갯수로 로드 결정하자
	return gamma_pdf;
}

float* set_version_pop(bitrate_version_set_info* info, int version_pop_type) {
	//Caching Strategies in Transcoding-enabled Proxy Systems for Streaming Media Distribution Networks
	//https://www.hpl.hp.com/techreports/2003/HPL-2003-261.pdf 기반임
	float* ver_pop = (float*)malloc(sizeof(float) * (info->version_num + 1));
	float mean;

	if (version_pop_type == HVP) {
		mean = info->version_num;
	}
	else if (version_pop_type == MVP) {
		if (info->version_num % 2 == 1)
			mean = (1 + info->version_num) / 2;
		else
			mean = info->version_num / 2;
	}
	else if (version_pop_type == LVP) {
		mean = 1;
	}
	else if (version_pop_type == RVP) {
		mean = (rand() % info->version_num) + 1;
	}

	float SD = SIGMA;
	// 모든 관련 논문들이 SD 값은 고정 fix값으로 하고, 값에 따라 달라지도록 시뮬레이션을 따로 하고 있다.
	// 나도 이거 그냥 실험에 추가 하는게 좋겠다.
	SD += ((SD * ((float)(rand() % 1001) / 10000)) - (SD * 0.05)); // += 5% 정도로 해서 SD만듦.

	//표준편차의 측정 단위는 원 자료와 같습니다. 예를 들면 원 자료의 측정 단위가 센티미터이면 표준편차의 측정 단위도 센티미터입니다.

	//이 편차가 작을 수록 우리 것에 불리. 왜냐하면 작을 수록 mean 버전에 인기도가 몰빵되기 때문.
	//A small σ indicates that most of the accesses are to one version(m) and a large σ indicates that the accesses to different version are evenly distributed.
	//작은 SD는 대부분의 액세스가 한 버전(m)에 대한 것이고 큰 SD는 다른 버전에 대한 액세스가 균등하게 분산되어 있음을 나타냅니다.

	float sum = 0;
	for (int ver = 1; ver <= info->version_num; ver++) {
		float result = 0;
		result = (1 / (sqrt(2 * PI) * SD)) * pow(M_E, -pow((ver - mean), 2) / (2 * pow(SD, 2)));

		ver_pop[ver] = result;
		sum += result;
	}
	for (int ver = 1; ver <= info->version_num; ver++) {
		ver_pop[ver] /= sum;
	}
	//모두 합해 합이 1이어야함

	return ver_pop;
}

void set_metric_score(bitrate_version_set_info* info, int metric_type) {
	//VMAF는 Qin, MMsys, Quality-aware Stategies for Optimizing ABR Video Streaming QoE and Reducing Data Usage, https://dl.acm.org/doi/pdf/10.1145/3304109.3306231 이 논문 기반임.
	//CBR 기준임
	mt19937 random_generation(SEED);
	float* mean = (float*)malloc(sizeof(float) * (info->version_num + 1)); // bitrate set에 따라 VMAF mean이 달라짐. 
	if (info->index == 0) { // zencoder 조합
		if (metric_type == VMAF) {
			mean[6] = 95; //2000 
			mean[5] = 92.5; //1500
			mean[4] = 90; //1000
			mean[3] = 72; //600
			mean[2] = 60; //400
			mean[1] = 40; //200
		}
		else if (metric_type == PSNR || metric_type == MOS) { // https://dl.acm.org/doi/pdf/10.1145/3304109.3306231 이 논문 기반임
			mean[6] = 41.5; //2000 
			mean[5] = 40; //1500
			mean[4] = 38.5; //1000
			mean[3] = 35; //600
			mean[2] = 33.5; //400
			mean[1] = 32; //200
		}
		else if (metric_type == SSIM) { //SSIM->MOS 변환
			//Light-weight Video Coding Based on Perceptual Video Quality for Live Streaming https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=8603274 
			mean[6] = 0.97; //2000 
			mean[5] = 0.96; //1500
			mean[4] = 0.95; //1000
			mean[3] = 0.93; //600
			mean[2] = 0.91; //400
			mean[1] = 0.87; //200
		}
	}
	//zencoder 빼고는 다 vmaf만 쓰므로 처리하지 않았음.
	else if (info->index == 1) { // youtube
		mean[10] = 99.9; //23500
		mean[9] = 99.5; //13500
		mean[8] = 98.6; //9500
		mean[7] = 98.4; //6750
		mean[6] = 98.2; //4500;
		mean[5] = 98; //4125;
		mean[4] = 96; //2750;
		mean[3] = 91.3; //1250;
		mean[2] = 77; //700;
		mean[1] = 65; //500;
	}
	else if (info->index == 2) { //netflix
		mean[9] = 98.1; //4300
		mean[8] = 97; //3000
		mean[7] = 95.5; //2350
		mean[6] = 93.8; // 1750
		mean[5] = 90.2; //1050
		mean[4] = 80; //750
		mean[3] = 67.5; //560
		mean[2] = 57.5; //375
		mean[1] = 43; // 235
	}
	else if (info->index == 4) { // 논문 https://doi.org/10.1145/3123266.3123426
		mean[8] = 98.3; //6000
		mean[7] = 97.8; //4000
		mean[6] = 97; // 3000
		mean[5] = 95.5; //2400
		mean[4] = 92.5; //1500
		mean[3] = 82; //807
		mean[2] = 65; //505
		mean[1] = 45; // 253
	}
	else if (info->index == 3) {// IBM Watson Media
		mean[6] = 98.3; //6000
		mean[5] = 96; // 2750
		mean[4] = 91.5; // 1350
		mean[3] = 91.5; // 1350
		mean[2] = 90; //1000
		mean[1] = 60; //400
	}

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		//normal_distribution<float> normal_distribution_for_metric[7]; //이거 어떻게 동적 배열로 바꿀지 생각하자
		/*normal_distribution<float> normal_distribution_for_metric_ver1;
		normal_distribution<float> normal_distribution_for_metric_ver2;
		normal_distribution<float> normal_distribution_for_metric_ver3;
		normal_distribution<float> normal_distribution_for_metric_ver4;
		normal_distribution<float> normal_distribution_for_metric_ver5;
		normal_distribution<float> normal_distribution_for_metric_ver6;*/
		//ver 7은 vmaf 100임
		float SD;
		if (metric_type == VMAF) {
			SD = rand() % 13 + 2; // 각 비디오의 표준 편차는 2~14의 값을 가짐
		}
		else if (metric_type == PSNR || metric_type == MOS) {
			SD = rand() % 14 + 41; //  4.1~5.4
			SD /= 10;
		} // https://doi.org/10.3390/s21061949
		else if (metric_type == SSIM) {
			SD = rand() % 23 + 4; // 0.004~0.026 vmaf 값 기반으로 scailing함. 도저히 없어서...
			SD /= 1000;
		}

		float temp = 0;
		for (int ver = 1; ver <= info->version_num; ver++) {
			normal_distribution<float> normal_distribution_for_metric(mean[ver], SD);
			if (ver == 1) {
				info->video_quality[channel][1] = normal_distribution_for_metric(random_generation);
				if (metric_type == VMAF) {
					while (info->video_quality[channel][1] > 100 || info->video_quality[channel][1] < 0) {
						info->video_quality[channel][1] = normal_distribution_for_metric(random_generation);
					}
				}
				else if (metric_type == PSNR || metric_type == MOS) {
					while (info->video_quality[channel][1] > 50 || info->video_quality[channel][1] < 0) {
						info->video_quality[channel][1] = normal_distribution_for_metric(random_generation);
					}
				}
				else if (metric_type == SSIM) {
					while (info->video_quality[channel][1] > 1 || info->video_quality[channel][1] < 0) {
						info->video_quality[channel][1] = normal_distribution_for_metric(random_generation);
					}
				}
			}
			else if (ver >= 2 && ver <= info->version_num - 1) {
				info->video_quality[channel][ver] = normal_distribution_for_metric(random_generation);
				if (metric_type == VMAF) {
					while (info->video_quality[channel][ver] <= info->video_quality[channel][ver - 1] || info->video_quality[channel][ver] > 100 || info->video_quality[channel][ver] < 0) {
						info->video_quality[channel][ver] = normal_distribution_for_metric(random_generation);
						if (temp == info->video_quality[channel][ver]) {
							channel--; // 21.04.27 지금 생각해보니 이렇게 하면 이전 채널의 괜찮았던 값 까지 다시 만들잖아? 싶지만 이미 이걸로 진행을 했기때문에, 일관성을 위해 그대로 씁니다...
							continue;
						}
						temp = info->video_quality[channel][ver];
					}
				}
				else if (metric_type == PSNR || metric_type == MOS) {
					while (info->video_quality[channel][ver] <= info->video_quality[channel][ver - 1] || info->video_quality[channel][ver] > 50 || info->video_quality[channel][ver] < 0) {
						info->video_quality[channel][ver] = normal_distribution_for_metric(random_generation);
						if (temp == info->video_quality[channel][ver]) {
							channel--; // 21.04.27 위의 주석 참고.....
							continue;
						}
						temp = info->video_quality[channel][ver];
					}
				}
				else if (metric_type == SSIM) {
					while (info->video_quality[channel][ver] <= info->video_quality[channel][ver - 1] || info->video_quality[channel][ver] > 1 || info->video_quality[channel][ver] < 0) {
						info->video_quality[channel][ver] = normal_distribution_for_metric(random_generation);
						if (temp == info->video_quality[channel][ver]) {
							channel--; // 21.04.27 위의 주석 참고.....
							continue;
						}
						temp = info->video_quality[channel][ver];
					}
				}
			}
			else if (ver == info->version_num) {
				if (metric_type == VMAF) {
					info->video_quality[channel][info->version_num] = 100;
				}
				else if (metric_type == PSNR || metric_type == MOS) { // https://stackoverflow.com/questions/39615894/handling-infinite-value-of-psnr-when-calculating-psnr-value-of-video
					info->video_quality[channel][info->version_num] = 43; // https://dl.acm.org/doi/pdf/10.1145/3304109.3306231 이 논문 기반임
				}
				else if (metric_type == SSIM) {
					info->video_quality[channel][info->version_num] = 1;
				}
			}
		}
	}
	if (metric_type == MOS) { // An ANFIS-based Hybrid Video Quality Prediction Model for Video Streaming over Wireless Networks
		for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
			for (int ver = 1; ver <= info->version_num; ver++) {
				float crt = info->video_quality[channel][ver];
				if (crt > 37) {
					info->video_quality[channel][ver] = 5.0;
				}
				else if (crt > 31) {
					info->video_quality[channel][ver] = 4.0;
				}
				else if (crt > 25) {
					info->video_quality[channel][ver] = 3.0;
				}
				else if (crt > 20) {
					info->video_quality[channel][ver] = 2.0;
				}
				else {
					info->video_quality[channel][ver] = 1.0;
				}
			}
		}
	}
}

void set_pop(bitrate_version_set_info* info, int version_pop_type) {
	float sum = 0;
	float* channel_pop = set_gamma_pop(CHANNEL_NUM, K_gamma, THETA_gamma);
	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		float* ver_pop = set_version_pop(info, version_pop_type);
		info->popularity[channel][0] = channel_pop[channel];
		for (int version = 1; version <= info->version_num; version++) {
			info->popularity[channel][version] = channel_pop[channel] * ver_pop[version];
			sum += info->popularity[channel][version];
		}
	}
}

void set_GHz(bitrate_version_set_info* info) {
	//채널 별로 GHz를 좀 다르게 줘보자.
	//MMsys Aparicio-Pardo의 Transcoding Live Adaptive Video Streams at a Massive Scale in the Cloud
	//CPU cycle 측정을 통한 GHz 계산한 결과를 테이블로 올려 둠. zecoder bitrate set 조합도 여기에 있음.
	float* a = (float*)malloc(sizeof(float) * (info->version_num));
	float* b = (float*)malloc(sizeof(float) * (info->version_num)); // 맨 위 버전은 트랜스코딩 원본으로 쓰므로 빠진 것에 유의할 것
	//int r[] = { 0, 200, 400, 600, 1000, 1500, 2000 };
	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		mt19937 random_generation(SEED);
		for (int version = 1; version <= info->version_num - 1; version++) {
			if (info->resolution[version] == 38402160) { //스케일링해서 구함
				a[version] = 2.597554426;
				b[version] = 0.184597645;
			}
			else if (info->resolution[version] == 25601440) { //스케일링해서 구함
				a[version] = 1.819367444;
				b[version] = 0.10754013;
			}
			else if (info->resolution[version] == 19201080) {
				a[version] = 1.547002;
				b[version] = 0.08057;
			}
			else if (info->resolution[version] == 1280720) {
				a[version] = 1.341512;
				b[version] = 0.060222;
			}
			else if (info->resolution[version] == 960540) { //스케일링해서 구함
				a[version] = 1.041912;
				b[version] = 0.044521;
			}
			else if (info->resolution[version] == 854480) { //스케일링해서 구함
				a[version] = 0.961305333;
				b[version] = 0.040296683;
			}
			else if (info->resolution[version] == 720480) { //스케일링해서 구함
				a[version] = 0.913512;
				b[version] = 0.037792;
			}
			else if (info->resolution[version] == 640480) { //스케일링해서 구함
				a[version] = 0.884978667;
				b[version] = 0.036296667;
			}
			else if (info->resolution[version] == 640360) {
				a[version] = 0.827912;
				b[version] = 0.033306;
			}
			else if (info->resolution[version] == 512384) { //스케일링해서 구함
				a[version] = 0.79075496;
				b[version] = 0.03122664;
			}
			else if (info->resolution[version] == 480270) { //스케일링해서 구함
				a[version] = 0.717074239;
				b[version] = 0.027103364;
			}
			else if (info->resolution[version] == 384288) { //스케일링해서 구함
				a[version] = 0.696173404;
				b[version] = 0.025933724;
			}
			else if (info->resolution[version] == 426240) { //스케일링해서 구함
				a[version] = 0.686989703;
				b[version] = 0.025419791;
			}
			else if (info->resolution[version] == 400224) {
				a[version] = 0.673091;
				b[version] = 0.024642;
			}
			else if (info->resolution[version] == 320240) { //스케일링해서 구함
				a[version] = 0.659016364;
				b[version] = 0.023854364;
			}

			float GHz = a[version] * pow(((double)info->bitrate[version] / 1000), b[version]);
			//float GHz = a[version] + pow(r[version], b[version]);
			GHz += ((GHz * ((float)(rand() % 1001) / 10000)) - (GHz * 0.05)); // += 10% 정도로 해서 GHz만듦.
			GHz *= 4; // 논문이 4 core 임. perf의 cycle 보니 저건 각 코어의 평균임. 
			/*if (GHz <= 0) {
				cout << "버그";
			}*/
			info->video_GHz[channel][version] = GHz;
		}
	}
}

void segment_init(bitrate_version_set_info* info, int version_pop_type, int metric_type) {
	//number_for_bit_opration = pow(2, info->version_num - 3);
	//set_versions_number = info->version_num - 2;

	srand(SEED);

	set_pop(info, version_pop_type);
	set_metric_score(info, metric_type);
	set_GHz(info);

	for (int channel = 1; channel <= CHANNEL_NUM; channel++) {
		//각 세트별 video pwq의 합을 구함.
		info->pwq[channel][1] = info->popularity[channel][1] * info->video_quality[channel][1]; //기본적으로 가장 낮은 버전은 반드시 포함되므로
		info->pwq[channel][info->version_num] = info->popularity[channel][info->version_num] * info->video_quality[channel][info->version_num]; //기본적으로 원본버전은 반드시 포함되므로

		for (int version = 1; version <= info->version_num; version++) {
			info->pwq[channel][version] = info->popularity[channel][version] * info->video_quality[channel][version];
		}

		for (int set = 1; set <= info->version_set_num; set++) { //소스는 전부 1080p
			info->video_quality_set[channel][set] += info->video_quality[channel][1];//기본적으로 가장 낮은 버전은 반드시 포함되므로
			info->video_quality_set[channel][set] += info->video_quality[channel][info->version_num];//기본적으로 원본버전은 반드시 포함되므로

			info->pwq_set[channel][set] += info->popularity[channel][1] * info->video_quality[channel][1]; //기본적으로 가장 낮은 버전은 반드시 포함되므로
			info->pwq_set[channel][set] += info->popularity[channel][info->version_num] * info->video_quality[channel][info->version_num]; //기본적으로 원본버전은 반드시 포함되므로

			info->sum_of_version_set_GHz[channel][set] += info->video_GHz[channel][1]; //기본적으로 가장 낮은 버전은 반드시 트랜스코딩 하므로

			int prev_version = 1; // set에 해당 버전이 없으면 그 아래 있는 버전을 스트리밍하므로
			for (int version = 2; version <= info->version_num - 1; version++) {
				if ((set - 1) & (info->number_for_bit_opration >> (info->set_versions_number_for_bit_opration - (version - 1)))) { //set에 해당 버전이 있는 경우

				// 16 >> ((info->version_num - 2) curr_ver) : curr_ver는 5~1이므로, 즉 10000을 0~4번 만큼 >> 쪽으로 시프트
				// 즉 (1)00001(1) (1)00010(1) (1)00100(1) (1)01000(1) (1)10000(1) 순
					info->video_quality_set[channel][set] += info->video_quality[channel][version];
					info->pwq_set[channel][set] += info->popularity[channel][version] * info->video_quality[channel][version];
					info->sum_of_version_set_GHz[channel][set] += info->video_GHz[channel][version];

					prev_version = version;
				}
				else {  //set에 해당 버전이 없는 경우
					info->video_quality_set[channel][set] += info->video_quality[channel][prev_version];
					info->pwq_set[channel][set] += info->popularity[channel][version] * info->video_quality[channel][prev_version];
				}
			}
		}
	}
}
int get_backend_max_GHz(int backend_node) {
	int bn_type = (backend_node - 1) % BN_TYPE_NUM + 1;
	return backend_max_GHz[bn_type];
}

float get_power(int backend_node, float GHz) {
	//단위는 W, 0~100% 까지 10의 단위로 있음
	int bn_type = (backend_node - 1) % BN_TYPE_NUM + 1;
	//[0]은 idle power, [1~10]은 active power. 

	//851	805	762	718	676	631	583	538	487	403

	float power_measure_list[BN_TYPE_NUM + 1][11] = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
						   { 111, 197, 221, 248, 280, 311, 335, 357, 378, 401, 422}, //ASUSTeK Computer Inc. RS700A-E9-RS4V2 
						   { 72.3, 181, 207, 229, 250, 270, 294, 321, 347, 370, 393 }, // Dell Inc. PowerEdge R7525
						   { 81.6, 188, 214, 236, 258, 280, 305, 331, 358, 382, 404},	// Dell Inc. PowerEdge R6525
						   { 123, 243, 289, 331, 376, 427, 495, 575, 663, 796, 894 }, // Hewlett Packard Enterprise ProLiant DL580 Gen10
							//새로 추가함
						   { 56.8, 100, 116, 130, 143, 162, 191, 211, 231, 250, 268 }, // Hewlett Packard Enterprise ProLiant DL325 Gen10
						   { 228, 492, 565, 627, 696, 785, 853, 929, 1001, 1071, 1140 }, //ASUSTeK Computer Inc. RS620SA-E10-RS12
						   { 131, 194, 221, 248, 274, 302, 336, 380, 444, 499, 580 }, //Fujitsu PRIMERGY RX2530 M6
						   { 523, 812, 940, 1053, 1161, 1360, 1605, 1768, 1928, 2097, 2260 }, //Hewlett Packard Enterprise Apollo XL225n Gen10 Plus
						   { 129, 257, 296, 333, 370, 413, 467, 532, 603, 688, 834 }, //Fujitsu PRIMERGY RX4770 M6
						   { 108, 171, 192, 215, 237, 256, 281, 304, 331, 382, 455 } }; //Lenovo Global Technology ThinkSystem SR665

	float percent = GHz / backend_max_GHz[bn_type];
	int index = percent / 0.1;
	/*if (bn_type == 1 &&  percent > 0.97) {
		cout << "디버그";
	}*/
	float active = 0;
	if (index < 10) {
		int temp = power_measure_list[bn_type][index + 1] - power_measure_list[bn_type][index];
		//0.95일 경우에는, 0.9 + <0.05> 반영이 필요함. 
		active = power_measure_list[bn_type][index] + temp * ((percent - (0.1 * index)) * 10);
	}
	else
		active = power_measure_list[bn_type][10];

	float idle = power_measure_list[bn_type][0];

	float power = active * percent + idle * (1.0f - percent);

	return power;
}
