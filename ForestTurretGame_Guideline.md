# Forest Turret Game Guideline

## 프로젝트 목표

`Forest_v01` 맵을 기반으로, 고정 터렛의 회전 레이저를 피하면서 총으로 터렛을 파괴하는 C++ 기반 Unreal Engine 게임을 만든다.

1차 목표는 MacBook에서 실행 가능한 완성 버전을 만드는 것이다. 최종 목표는 Apple Vision Pro용 visionOS 빌드까지 확장하는 것이다.

현재 프로젝트 기준:

- Unreal Engine: 5.6
- C++ 모듈: `RussellTutorial`
- 목표 맵: `Content/ThirdPersonBP/Maps/Forest_v01.umap`
- 현재 입력 설정: 기존 Third Person 입력 + Enhanced Input 클래스 참조
- 현재 C++ 상태: 기본 `MyActor` 클래스만 존재

## 핵심 게임 방식

- 터렛은 맵에 고정되어 있다.
- 터렛 머리에서 빨간색 레이저가 나온다.
- 터렛 머리는 계속 360도로 회전한다.
- 플레이어가 레이저에 맞으면 사망한다.
- 플레이어가 총을 발사해 터렛을 맞추면 터렛이 파괴된다.
- 터렛이 파괴되면 게임 클리어 상태가 된다.

## 개발 원칙

- 게임 규칙은 C++ 중심으로 구현한다.
- 메시, 머티리얼, 회전 속도, 레이저 길이 같은 튜닝 값은 Blueprint 자식 클래스에서 조정 가능하게 둔다.
- `Forest_v01` 원본 맵은 보존하고, 작업용 복사본을 만들어 사용한다.
- MacBook 실행 버전을 먼저 완성한 뒤 Vision Pro 대응을 별도 단계로 진행한다.
- Vision Pro를 고려해 입력, 카메라, 게임 규칙을 서로 강하게 묶지 않는다.

## 추천 맵 구성

원본 맵:

- `Content/ThirdPersonBP/Maps/Forest_v01.umap`

작업용 맵:

- `Content/ThirdPersonBP/Maps/Forest_Turret_MVP.umap`

권장 흐름:

1. `Forest_v01`을 복사해 `Forest_Turret_MVP`를 만든다.
2. 플레이어 시작 위치를 정한다.
3. 터렛 배치 지점을 정한다.
4. 터렛과 플레이어 사이에 레이저를 피할 수 있는 엄폐 또는 이동 경로가 있는지 확인한다.
5. 레이저가 나무, 지형, 소품에 막힐지 여부를 의도에 맞게 정한다.

## C++ 클래스 설계

### `ATurretActor`

터렛 본체를 담당한다.

주요 컴포넌트:

- `USceneComponent* Root`
- `UStaticMeshComponent* BaseMesh`
- `UStaticMeshComponent* HeadMesh`
- `USceneComponent* LaserOrigin`
- `UStaticMeshComponent* LaserVisual` 또는 Niagara beam component

주요 변수:

- `float RotationSpeedDegreesPerSecond`
- `float LaserRange`
- `float LaserTraceRadius`
- `int32 MaxHealth`
- `int32 CurrentHealth`
- `bool bIsDestroyed`

주요 함수:

- `Tick(float DeltaTime)`: 터렛 머리 회전 및 레이저 판정
- `UpdateLaser(float DeltaTime)`: 레이저 trace 실행
- `ApplyTurretDamage(int32 DamageAmount)`: 터렛 피격 처리
- `DestroyTurret()`: 파괴 연출 및 클리어 알림

레이저 판정은 처음에는 `SphereTraceSingleByChannel`을 권장한다. 완전한 `LineTrace`는 판정이 너무 얇아 플레이 경험이 불안정할 수 있다.

### `ARussellCharacter`

플레이어 조작과 발사를 담당한다.

주요 기능:

- 이동
- 시점 조작
- 점프
- 발사
- 사망 처리

발사 방식:

- MVP에서는 히트스캔 방식의 `LineTrace`를 사용한다.
- 카메라 또는 총구 위치에서 정면 방향으로 trace를 쏜다.
- 터렛에 맞으면 `ATurretActor::ApplyTurretDamage()`를 호출한다.

나중에 총알 궤적, 탄속, 낙차가 필요해지면 `AProjectile` 클래스로 확장한다.

### `ARussellPlayerController`

입력과 플레이어 상태 전환을 관리한다.

주요 기능:

- Enhanced Input mapping context 등록
- 발사 입력 처리
- 사망 후 재시작 입력 처리
- 게임 클리어 후 입력 제한

### `ARussellGameMode`

게임 흐름을 관리한다.

주요 기능:

- 플레이어 사망 처리
- 레벨 재시작
- 터렛 파괴 시 승리 처리
- HUD 또는 UI 상태 전환

### 선택 클래스: `UHealthComponent`

터렛 하나만 등장하는 MVP에서는 필수는 아니다. 하지만 이후 적, 파괴 가능한 오브젝트, 보스 터렛 등이 늘어날 수 있다면 체력 처리를 컴포넌트로 분리한다.

## 입력 설계

현재 프로젝트에는 기존 Action/Axis mapping과 Enhanced Input 클래스 참조가 같이 존재한다. C++ 구현 단계에서는 `RussellTutorial.Build.cs`에 `EnhancedInput` 모듈을 추가하는 것을 권장한다.

필수 입력:

- `Move`
- `Look`
- `Jump`
- `Fire`
- `Restart`

MacBook MVP 입력:

- 이동: WASD
- 시점: Mouse X/Y
- 발사: Left Mouse Button
- 점프: Space
- 재시작: R

Vision Pro 전환 시 입력 후보:

- 시선 조준 + 손 제스처 발사
- 컨트롤러 방향 조준 + 트리거 발사
- 헤드 방향 조준 + 핀치 발사

중요한 점은 게임 규칙이 특정 입력 장치에 직접 의존하지 않도록 만드는 것이다. 예를 들어 터렛 피격 함수는 마우스, 컨트롤러, 손 제스처 어디에서 호출되더라도 같은 방식으로 동작해야 한다.

## 터렛 레이저 구현 가이드

레이저는 시각 효과와 충돌 판정을 분리한다.

시각 효과:

- 빨간 emissive material
- 얇은 실린더 mesh
- Niagara beam
- 길이와 두께는 Blueprint에서 조정 가능하게 노출

충돌 판정:

- `LaserOrigin`에서 `HeadMesh`의 forward vector 방향으로 trace
- 기본은 `SphereTraceSingleByChannel`
- trace 대상은 플레이어 pawn 또는 플레이어 collision channel

추천 초기값:

- 회전 속도: 초당 45도
- 레이저 길이: 5000 Unreal units
- 레이저 판정 반경: 8~20 Unreal units
- 터렛 체력: 3
- 플레이어 발사 데미지: 1

튜닝 방향:

- 너무 어렵다면 회전 속도를 낮춘다.
- 레이저가 맞았는지 애매하면 시각 빔 두께와 trace radius를 맞춘다.
- 숲 오브젝트가 판정을 너무 많이 막으면 레이저 충돌 채널을 분리한다.

## 게임 상태

권장 상태:

- `Playing`
- `PlayerDead`
- `TurretDestroyed`
- `GameClear`

플레이 중:

- 플레이어 이동 가능
- 발사 가능
- 터렛 회전 및 레이저 판정 활성

플레이어 사망:

- 플레이어 입력 제한
- 사망 UI 또는 카메라 연출
- `R` 입력 또는 일정 시간 후 레벨 재시작

터렛 파괴:

- 터렛 레이저 비활성화
- 파괴 이펙트 또는 메시 숨김
- 클리어 UI 표시

## MVP 완료 기준

아래 항목이 모두 동작하면 1차 MVP 완료로 본다.

- `Forest_Turret_MVP` 맵에서 게임 시작 가능
- 플레이어가 이동하고 시점을 조작 가능
- 터렛 머리가 계속 360도 회전
- 빨간 레이저가 터렛 머리 방향으로 표시됨
- 레이저에 맞으면 플레이어가 사망
- 마우스 클릭으로 총 발사 가능
- 총 trace가 터렛을 맞추면 터렛 체력 감소
- 체력이 0이 되면 터렛 파괴
- 터렛 파괴 후 게임 클리어 상태 진입
- 사망 또는 클리어 후 재시작 가능

## MacBook 실행 목표

MacBook 실행 버전은 MVP 완성 후 검증한다.

필수 환경:

- Apple silicon 또는 Intel MacBook
- Unreal Engine 5.6
- Xcode
- Command Line Tools

검증 절차:

1. 프로젝트를 MacBook으로 이동한다.
2. `.uproject`를 Unreal Engine 5.6으로 연다.
3. C++ 모듈을 재빌드한다.
4. 에디터에서 `Forest_Turret_MVP`를 실행한다.
5. macOS 패키징을 수행한다.
6. 패키징된 앱을 직접 실행한다.

완료 기준:

- MacBook에서 에디터 실행 가능
- MacBook에서 패키징 성공
- 패키징된 앱에서 게임 루프 정상 동작

## Apple Vision Pro 확장 목표

Vision Pro 대응은 MacBook 버전 완성 후 별도 마일스톤으로 진행한다.

전환 대상:

- XR 카메라
- XR 입력
- VisionOS/OpenXR 플러그인
- 모바일/XR 렌더링 최적화
- Apple Developer signing/provisioning
- visionOS 패키징

게임 설계 변경 후보:

- Third-person 카메라를 XR 시점으로 변경
- 이동 방식을 순간이동, 고정 플레이 공간, 짧은 이동 중 하나로 선택
- 조준 방식을 시선, 손, 컨트롤러 중 하나로 선택
- 발사를 핀치 또는 트리거 입력에 연결
- 레이저 피격 판정을 플레이어 capsule 기준에서 HMD 기준으로 조정할지 검토

Vision Pro 주의사항:

- 멀미 방지를 위해 강제 카메라 이동을 피한다.
- 레이저, 총구, UI는 깊이감과 실제 크기를 고려해 배치한다.
- 숲 맵의 고품질 에셋은 성능 부담이 클 수 있으므로 LOD, shadow, post process를 점검한다.
- MacBook 버전에서 작성한 C++ 게임 규칙을 최대한 유지하고, 플랫폼별 입력과 카메라만 교체한다.

## 마일스톤

### M0: 프로젝트 정리

- `Forest_v01` 복사본 생성
- C++ 클래스 네이밍 정리
- Enhanced Input 모듈 추가
- 기본 GameMode/Character/Controller 구성

완료 기준:

- 새 맵에서 C++ GameMode로 플레이 가능

### M1: 터렛 MVP

- `ATurretActor` 생성
- 터렛 머리 회전 구현
- 빨간 레이저 시각화
- 레이저 trace 구현

완료 기준:

- 터렛이 회전하고 플레이어 피격을 감지

### M2: 플레이어 발사

- `Fire` 입력 추가
- 히트스캔 trace 구현
- 터렛 체력 감소 구현
- 터렛 파괴 처리

완료 기준:

- 플레이어가 총으로 터렛을 파괴 가능

### M3: 게임 루프

- 플레이어 사망 처리
- 재시작 처리
- 게임 클리어 처리
- 간단한 UI 추가

완료 기준:

- 시작, 사망, 재시작, 클리어 흐름이 모두 연결됨

### M4: MacBook 검증

- MacBook에서 프로젝트 열기
- C++ 재빌드
- macOS 패키징
- 패키징 앱 실행

완료 기준:

- MacBook에서 완성된 게임 실행 가능

### M5: Vision Pro 프로토타입

- VisionOS/OpenXR 환경 설정
- XR 카메라 테스트
- XR 입력 테스트
- 별도 테스트 맵에서 빌드 검증

완료 기준:

- Vision Pro 또는 visionOS 시뮬레이터에서 기본 상호작용 확인

### M6: Vision Pro 게임화

- Forest 맵 최적화
- XR 조준/발사 연결
- XR 피격/클리어 흐름 조정
- 실제 기기 빌드 테스트

완료 기준:

- Apple Vision Pro에서 게임 루프 플레이 가능

## 구현 시작 순서

가장 먼저 구현할 항목:

1. `ATurretActor`
2. 플레이어 `Fire` 입력
3. 터렛 피격/파괴
4. 플레이어 사망
5. 게임 클리어/재시작

이 순서가 좋은 이유는 게임의 핵심 재미를 가장 빨리 확인할 수 있기 때문이다. UI, 이펙트, Vision Pro 대응은 핵심 루프가 손에 잡힌 뒤 진행한다.

## 참고 링크

- Unreal Engine 5.6 Supported XR Devices: https://dev.epicgames.com/documentation/en-us/unreal-engine/supported-xr-devices-in-unreal-engine?application_version=5.6
- Unreal Engine 5.6 Release Notes: https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-6-release-notes
- Unreal Engine 5.6 iOS/iPadOS/tvOS Requirements: https://dev.epicgames.com/documentation/en-us/unreal-engine/ios-ipados-and-tvos-development-requirements-for-unreal-engine?application_version=5.6
- Apple Xcode System Requirements: https://developer.apple.com/xcode/system-requirements/
