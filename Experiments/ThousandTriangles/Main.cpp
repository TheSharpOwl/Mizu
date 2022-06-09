//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#include "stdafx.h"
#include "ThousandTriangles.h"

// Indicates to hybrid graphics systems to prefer the discrete part by default
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	float s[] = {
		1.0f
		,0.01f
		,0.015f
		,0.02f
		,0.029f
		,0.043f
		,0.062f
		,0.0905f
	};

    // for one pixel triangle 0.01 thrid parameter
    ThousandTriangles sample(1280, 720, s[7]);
    return Win32Application::Run(&sample, hInstance, nCmdShow);
}
