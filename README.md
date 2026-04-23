# ZombieShooting

> Unreal Engine `5.6.1` 기반 C++ 1인칭 좀비 슈터 프로토타입입니다. 기존 `Forest_v01` 맵 위에서 빠르게 플레이 가능한 생존 루프를 만드는 것을 목표로 합니다.

## 프로젝트 개요

`ZombieShooting`은 기존 숲 맵을 유지한 채 에디터에서 바로 플레이 가능한 최소 완성 루프를 구현하는 프로젝트입니다.
핵심 게임플레이는 C++ 중심으로 작성했고, 이후 MacBook 환경과 Apple Vision Pro 확장을 고려해 입력, 전투, HUD, AI, 게임 흐름을 비교적 분리된 구조로 유지하고 있습니다.

## 현재 구현된 플레이 루프

- `/Game/ThirdPersonBP/Maps/Forest_v01`에서 플레이어 시작
- 1인칭 카메라와 키보드/마우스 이동
- 히트스캔 샷건 발사
- 일정 시간 뒤 등장하는 RPG7 무기 픽업
- NavMesh 기반 좀비 추적과 근접 공격
- 좀비 피격, 사망, 킬 카운트
- 플레이어 체력 감소, 사망, 레벨 재시작
- 체력, 탄약, 현재 무기, 웨이브, 킬 수, 생존 좀비 수, FPS를 표시하는 HUD
- 기본 좀비 외 ghoul, skeleton, lich 기반 파생 변종 포함

## 기술 스택

- Unreal Engine `5.6.1`
- C++ gameplay module: `Source/ZombieShooting`
- 기본 맵: `/Game/ThirdPersonBP/Maps/Forest_v01`
- 주요 런타임 플러그인: `EnhancedInput`, `Niagara`
- `PythonScriptPlugin`은 `Editor` 전용으로 제한

## 핵심 코드 구조

- `Source/ZombieShooting/ZombiePlayerCharacter.*`
  1인칭 플레이어, 입력 처리, 카메라, 무기 장착/교체
- `Source/ZombieShooting/WeaponComponent.*`
  샷건/RPG7 발사, 탄약, 발사 간격, FX
- `Source/ZombieShooting/CombatHealthComponent.*`
  플레이어/적 공용 체력 처리
- `Source/ZombieShooting/ZombieCharacter.*`
  좀비 이동, 추적, 공격, 피격/사망, 변종 애니메이션
- `Source/ZombieShooting/ZombieShootingGameMode.*`
  웨이브 스폰, 킬 집계, 재시작, 성능 프로파일 적용
- `Source/ZombieShooting/SurvivalHUD.*`
  캔버스 기반 HUD와 FPS 표시

## 실행 방법

### 요구 사항

- Unreal Engine `5.6.1`
- Windows 개발 시 Visual Studio 2022 C++ 툴체인
- macOS 실행 메모: [Build/Mac/README.md](Build/Mac/README.md)

### 에디터에서 바로 실행

1. 저장소를 클론합니다.
2. `ZombieShooting.uproject`를 Unreal Engine `5.6.1`로 엽니다.
3. 모듈 재빌드 여부를 묻는 창이 나오면 `Yes`를 선택합니다.
4. 맵이 자동으로 열리지 않으면 `/Game/ThirdPersonBP/Maps/Forest_v01`를 엽니다.
5. 에디터에서 `Play`를 누릅니다.

## 기본 조작

- 이동: `W`, `A`, `S`, `D`
- 시점: 마우스
- 점프: `Space`
- 발사: `Left Mouse Button`
- 재장전: `R`
- 재시작: `Enter`

기본 샷건은 현재 무한 탄약 설정으로 동작하며, `Reload` 입력은 이후 유한 탄약 구성까지 고려해 유지하고 있습니다.

## 저장소에서 보면 좋은 위치

- `PROJECT_PROGRESS.md`
  작업 히스토리, 검증 로그, 남은 이슈를 정리한 핸드오프 문서
- `Config/DefaultEngine.ini`
  기본 맵, 게임모드, macOS 관련 설정
- `Config/DefaultInput.ini`
  키보드/마우스 입력 매핑
- `Build/Mac/README.md`
  MacBook Air 기준 첫 실행 및 패키징 메모

## 현재 상태

- 2026-04-24 기준 `Win64 Development`와 `ZombieShootingEditor Win64 Development` 빌드가 성공했습니다.
- 에디터 플레이 기준 최소 생존 루프는 구현되어 있습니다.
- 일부 시각 연출, 픽업 상호작용감, 변종 애니메이션 자연스러움, 전경 기준 성능 측정은 추가 수동 검증이 남아 있습니다.
- 로컬 워크스페이스 폴더명은 여전히 `RussellTutorial`이지만, 실제 Unreal 프로젝트와 모듈 이름은 `ZombieShooting`입니다.

## 에셋 및 배포 메모

이 프로젝트는 기존 맵과 이미 프로젝트에 포함된 Marketplace/Fab 계열 에셋을 재사용합니다. 공개 저장소 운영이나 외부 배포를 진행할 때는 사용 중인 에셋의 라이선스 및 재배포 조건을 별도로 확인하는 것을 권장합니다.

## 다음 목표

- 전경 PIE 기준 오프닝 웨이브 성능 재확인
- 좀비 변종의 피격/이동 연출 다듬기
- HUD/UI와 FX 완성도 향상
- macOS 및 향후 Apple Vision Pro 확장을 위한 구조 정리
