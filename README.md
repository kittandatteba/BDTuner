# BDTuner

## ����͉��H
�u���h�b�O�E�`���[�i�[����AKB-PCR20X�p���e��A�v���P�[�V��������g�p���邽�߂̃��C�u�����iDLL�j�ł��B  
PT3-SDK�iSDK_EARTHSOFT_PT3.dll�j�Ƃ��ĐU�镑�����߁APT3�p�A�v���P�[�V�����iptTimer�ABonDriver_PT3-ST�Ȃǁj����u���h�b�O�E�`���[�i�[���g�p�\�ɂȂ�܂��B  

## ��������
�ȉ��̐�������������܂��B
- AKB-PCR20X��ISDB-S 1TS��ISDB-T 1TS�̍��v2TS�̂��߁A1��ڑ�����PT3-SDK��2�ڂ̃`���[�i�[�͎�M�s�\��ԁi�A���e�i���q�����Ă��Ȃ���ԂƓ����j�ɂȂ�܂��B  
AKB-PCR20X��2��ڑ������ꍇ�́APT3�Ɠ��l��ISDB-S 2TS��ISDB-T 2TS�̍��v4TS�𓯎��Ɏ�M�\�ł��B�i������ڑ��̍ۂɃV���A��ROM��ID�ԍ����������͕s�v�ł��B�j  
- �A�v���P�[�V�����N������USB�̔��������ɂ͑Ή����Ă��܂���B
- LNB���d�͂ł��܂���B�i���������n�[�h�E�F�A�Ɏ�������Ă��Ȃ����߁B�j

## ���e
���|�W�g���͈ȉ��̍\���ɂȂ��Ă��܂��B
* src\PT3SDKSimulator  
PT3-SDK�iSDK_EARTHSOFT_PT3.dll�j���V�~�����[�g����DLL�ł��B  
PT3-SDK�̃C���^�[�t�F�[�X�̂����A�Œ���K�v�ȋ@�\�ɍi���Ď��������Ă��܂��B  
Microsoft Visual Studio 2017�̃\�����[�V�����`���ɂȂ��Ă��܂��B

* src\CPLDProgrammer  
CPLD��MAX�U�Ƀ��W�b�N���������ނ��߂̃c�[���ł��B  
���̓t�@�C����SVF�`���ł��B  
CUSBWR2.exe��CyUSB3�h���C�o�Ƃ̑g�ݍ��킹�Ő��������삷�邩���O�����������߁A�Ǝ��ɏ������݃c�[�������܂����B  
Microsoft Visual Studio 2017�̃\�����[�V�����`���ɂȂ��Ă��܂��B

* uvision\tuner�Auvision\bitbang  
EZ-USB FX2LP�̃t�@�[���E�G�A�ł��B
tuner��PT3SDKSimulator�p�Abitbang��CPLDProgrammer�p�ł��B  
CY3684 EZ-USB FX2LP Development Kit�t����Keil ��Vision�̃v���W�F�N�g�`���ɂȂ��Ă��܂��B

* quatus  
MAX�U�̃��W�b�N�ł��B  
Intel Quatus Prime�̃v���W�F�N�g�`���ɂȂ��Ă��܂��B

* external  
�O���w�b�_�[�t�@�C���A���C�u�����̊i�[�ꏊ�ł��B

## �r���h���@
**32bit��**��DLL����уA�v���P�[�V�����̍쐬�菇�ł��B

* PT3-SDK�w�b�_�[�t�@�C���̓���ƓW�J  
PT3-SDK�̃\�[�X�R�[�h���_�E�����[�h���܂��B
�����_�ł�[������](https://earthsoft.jp/PT3/download_old.html) ����uPT3-SDK-300.zip�v���_�E�����[�h�\�ł��B  
zip�t�@�C�����́uEARTH_PT3.h�v�uPrefix.h�v�uPrefix_Bit.h�v�uPrefix_Integer.h�v��4�t�@�C�����uBDTuner\external\inc�v�ɔz�u���܂��B  
EARTH_PT3.h�́AVisual Studio 2017�ŃR���p�C������Ɓunot�v�Ƃ����ϐ������\���Ƃ̋����ŃG���[�ɂȂ邽�߁A�Y���s���ȉ��̂悤�ɏ��������܂��B  
```c
    //virtual status SetTransferTestMode(ISDB isdb, uint32 tunerIndex, bool testMode = false, uint16 initial = 0, bool not = false/*, bool resetError = false*/) = 0;
    virtual status SetTransferTestMode(ISDB isdb, uint32 tunerIndex, bool testMode = false, uint16 initial = 0, bool _not = false/*, bool resetError = false*/) = 0;
```

* CyAPI3�w�b�_�[�t�@�C���ƃ��C�u�����̓���ƓW�J  
CyAPI3���C�u�������_�E�����[�h���܂��B
�����_�ł�[������](http://www.cypress.com/documentation/software-and-drivers/ez-usb-fx3-software-development-kit) ����ucy_ssusbsuite_v1.3.3_0.zip�v���_�E�����[�h�\�ł��B  
zip�t�@�C�����́uEZ-USB FX3 SDK\1.3\library\cpp\inc�v�ɂ���uCyAPI.h�v�uCyUSB30_def.h�v��2�t�@�C�����uBDTuner\external\inc�v�ɔz�u���܂��B  
���l�ɁAzip�t�@�C�����́uEZ-USB FX3 SDK\1.3\library\cpp\lib\x86�v�ɂ���uCyAPI.lib�v���uBDTuner\external\lib\x86�v�ɔz�u���܂��B  

* �r���h  
Visual Studio�ŁuBDTuner\src�v�ɂ���uBDTuner.sln�v���J���A�u�\�����[�V�����̃r���h�v�����s���܂��B  
��������ƁuBDTuner\src\Release�iDebug�r���h�̏ꍇ��BDTuner\src\Debug�j�v�ɁA�uSDK_EARTHSOFT_PT3.dll�v�ƁuCPLDProgrammer.exe�v���쐬����܂��B

## �����菇
�{�\�t�g�E�F�A���g�p���邽�߂̓����菇�ł��B
* CyUSB3�h���C�o�̓���ƃC���X�g�[��   
CyUSB3�h���C�o���_�E�����[�h���A�g�pOS�ɍ������h���C�o���C���X�g�[�����܂��B
�����_�ł�[������](https://community.cypress.com/docs/DOC-12366) ����uDrivers.zip�v���_�E�����[�h�\�ł��B  

* �`���[�i�[�{�̂�PC�ɐڑ�  
AKB-PCR20X�{�̂�t����USB�P�[�u����PC�ɐڑ����܂��B

* MAX�U�փ��W�b�N����������  
�uCPLDProgrammer.exe�v���g�p����MAX�U�փ��W�b�N���������݂܂��B  �w�肷��t�@�C���́uBDTuner\quatus\output_files_sample�v�ɂ���utuner.svf�v�ł��B  
�uBDTuner\src\Release�v�z����Power Shell�܂��̓R�}���h�v�����v�g���J���A�ȉ��̃R�}���h�����s���܂��B  
`
CPLDProgrammer.exe ..\..\quatus\output_files_sample\tuner.svf
`  
CPLDProgrammer�͏������ݑ��x��ǋy���Ă��Ȃ����߁A�����܂�30���قǂ�����܂��B�r���o�߂Ƃ��Ď��s����SVF�̖��߂��W���o�͂ɏo�͂���܂��B�Ō�Ɂufinished normal�v�Əo�͂����ΐ����ł��B  
���W�b�N���������݂̍ہAPC�ɐڑ�����`���[�i�[�{�̂�**1��̂�**�ɂ��Ă��������B

## �g�p���@
�{�\�t�g�E�F�A�̎g�p���@�ł��B  
* ��{�I�ɂ́A�g�p����A�v���P�[�V������.exe�t�@�C��������f�B���N�g���ɁuSDK_EARTHSOFT_PT3.dll�v��z�u����Ύg�p�ł���͂��ł��B  
�������AWindows�̃V�X�e���f�B���N�g���ɂ���PT3-SDK��DLL�t�@�C���݂̂��Q�Ƃ���A�v���P�[�V�����iptTimer�Ȃǁj�ł́u%SystemRoot%\SysWOW64�iWindowsOS��32bit�ł̏ꍇ��%SystemRoot%\System32�j�v�ɁuSDK_EARTHSOFT_PT3.dll�v��z�u����K�v������܂��B

## �Q�l����
�쐬�ɂ������āA�ȉ��̃\�t�g�E�F�A�𗘗p�E�Q�Ƃ����Ē����܂����B ���肪�Ƃ��������܂��B
* �f�W���Y�� cap_sts_ 2008_03_18��
* �A�[�X�\�t�g PT3-SDK �\�[�X�R�[�h
* Cypress CY3684 EZ-USB FX2LP Development Kit �T���v���R�[�h
* ���c���a�I�� USB CPLD Programmer
