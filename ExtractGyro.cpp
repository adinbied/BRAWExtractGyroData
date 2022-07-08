#include "BlackmagicRawAPIDispatch.h"

#include <stdio.h>
#include <iostream>

#ifdef _DEBUG
	#include <cassert>
	#define VERIFY(condition) assert(SUCCEEDED(condition))
#else
	#define VERIFY(condition) condition
#endif

static const char* const s_outputFileName = "outputgyro.wav";

int main(int argc, const char* argv[])
{
	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " clipName.braw" << std::endl;
		return 1;
	}

	BSTR clipName;
	bool clipNameProvided = argc == 2;
	if (clipNameProvided)
	{
		const char* str = argv[1];
		int strLength = (int)strlen(str);
		int wslen = MultiByteToWideChar(CP_ACP, 0, str, strLength, 0, 0);
		clipName = SysAllocStringLen(0, wslen);
		MultiByteToWideChar(CP_ACP, 0, argv[1], strLength, clipName, wslen);
	}
	else
	{
		clipName = SysAllocString(L"../../../Media/sample.braw");
	}

	HRESULT result = S_OK;

	IBlackmagicRawFactory* factory = nullptr;
	IBlackmagicRaw* codec = nullptr;
	IBlackmagicRawClip* clip = nullptr;
	IBlackmagicRawClipGyroscopeMotion* gyro = nullptr;
	float rate;
	uint32_t count;

	do
	{
		result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (result != S_OK)
		{
			std::cerr << "Initialization of COM failed!" << std::endl;
			break;
		}

		BSTR libraryPath = SysAllocString(L"../../Libraries");
		factory = CreateBlackmagicRawFactoryInstanceFromPath(libraryPath);
		SysFreeString(libraryPath);
		if (factory == nullptr)
		{
			std::cerr << "Failed to create IBlackmagicRawFactory!" << std::endl;
			break;
		}

		result = factory->CreateCodec(&codec);
		if (result != S_OK)
		{
			std::cerr << "Failed to create IBlackmagicRaw!" << std::endl;
			break;
		}

		result = codec->OpenClip(clipName, &clip);
		if (result != S_OK)
		{
			std::cerr << "Failed to open IBlackmagicRawClip!" << std::endl;
			break;
		}

		result = clip->QueryInterface(IID_IBlackmagicRawClipGyroscopeMotion, (void**)&gyro);
		if (result != S_OK)
		{
			std::cerr << "Failed to get IBlackmagicRawClipgyro!" << std::endl;
			break;
		}


		result = gyro->GetSampleRate(&rate);
		result = gyro->GetSampleCount(&count);
		uint32_t sampleSize;

		if (gyro->GetSampleSize(&sampleSize) != S_OK) {
			
			std::cerr << "Failed to get sample size\n";

			exit(0);

		}

		float* samples = new float[sampleSize];

		uint32_t returnedSampleCount;
		std::cout << "sample,subsample,data" << "\n";
		for (uint64_t sample = 0; sample < count; sample++) {

			if (gyro->GetSampleRange(sample, 1, samples, &returnedSampleCount) != S_OK) {

				std::cerr << "Failed to get sample range for sample number " << sample << "\n";
				std::cout << "sampleSize " << sampleSize << "\n";

				exit(0);

			}
			
			for (int i = 0; i < sampleSize; i++)
				std::cout << "Sample " << sample << ", Sub-Sample " << i+1 << ": " << samples[i] << "\n";
		}
		} while (0);

	
	if (gyro != nullptr)
		gyro->Release();

	if (clip != nullptr)
		clip->Release();

	if (codec != nullptr)
		codec->Release();

	if (factory != nullptr)
		factory->Release();

	CoUninitialize();

	SysFreeString(clipName);

	return result;
}
