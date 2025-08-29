# BMP 이미지 처리 소프트웨어 (그레이스케일, 엣지 검출, MEM 생성)

<img width="315" height="315" alt="brainct_001" src="https://github.com/user-attachments/assets/8d03445f-322d-4cc2-9d90-251370a270b4" />

Grayscale image


<img width="315" height="315" alt="output_edge.bmp" src="https://github.com/user-attachments/files/22039849/output_edge.bmp" />


Edge image

## 문서 관리 정보
 - 문서 ID: BMP-DO178C-003
 - 버전: 2.1 / 날짜: 2025년 8월 29일
 - 분류: 설계 보증 레벨 D (DAL-D)
 - 시스템: 의료용 반도체 검증 환경용

### 1. 소프트웨어 인증 계획서 (PSAC)
#### 1.1 소프트웨어 개요
 - 소프트웨어 항목: BMP 이미지 프로세서
 - 기능: 24비트 컬러 BMP 이미지를 8비트 그레이스케일로 변환하고, 소벨 필터를 이용해 엣지를 검출하며, Verilog 시뮬레이션용 16진수 텍스트(.mem) 파일을 생성한다.
 - 중요도 레벨: DAL-D (경미한 고장 상황)

### 2. 소프트웨어 요구사항 표준 (SRS)
#### 2.1 상위레벨 요구사항
 * HLR-001 : 이미지 형식 및 크기 검증
   * 소프트웨어는 입력 파일이 24비트 BMP 형식이며, 이미지 크기가 630x630 픽셀인지 확인해야 한다.
 * HLR-002 : 색공간 변환
   * 소프트웨어는 표준 휘도 공식을 사용하여 RGB 픽셀을 그레이스케일로 변환해야 한다.
 * HLR-003 : 엣지 검출
   * 소프트웨어는 그레이스케일 이미지에 소벨 연산자를 적용하여 엣지를 검출해야 한다.
 * HLR-004 : 출력 파일 생성
   * 소프트웨어는 그레이스케일, 엣지 검출 결과를 각각 8비트 BMP 파일로 생성해야 한다.
   * 소프트웨어는 그레이스케일 데이터를 16진수 텍스트 형식의 MEM 파일로 생성해야 한다.
 * HLR-005 : 메모리 관리
   * 소프트웨어는 동적으로 할당된 모든 메모리를 해제해야 한다.
 * HLR-006 : 오류 처리
   * 소프트웨어는 파일 입출력, 메모리 할당 실패 등의 오류를 감지하고 처리해야 한다.

#### 2.2 저수준 요구사항
 * LLR-001 : BMP 헤더 검증
   * 파일 시그니처(0x4D42), bitCount(24), width(630), height(630)를 확인한다.
 * LLR-002 : 픽셀 데이터 읽기
   * 파일 헤더의 오프셋을 기준으로, 각 행의 패딩을 고려하여 이미지 픽셀 데이터를 읽는다.
 * LLR-003 : 그레이스케일 변환 알고리즘
   * Y = 0.299*R + 0.587*G + 0.114*B 공식을 적용하고 결과를 uint8_t로 캐스팅한다.
 * LLR-004 : 소벨 필터 알고리즘
   * 3x3 소벨 X, Y 커널을 사용하여 각 픽셀의 x, y 방향 그래디언트(Gx, Gy)를 계산한다.
   * 엣지 강도를 sqrt(Gx^2 + Gy^2)로 계산하고, 결과가 255를 넘지 않도록 클리핑한다.
 * LLR-005 : 출력 파일 생성
   * 8비트 그레이스케일 형식에 맞는 표준 BMP 헤더를 생성하여 각 결과(그레이스케일, 엣지)를 파일로 저장한다.
   * 그레이스케일 픽셀 값을 16진수 텍스트로 변환하여 MEM 파일에 저장한다.

### 3. 소프트웨어 설계 표준 (SDS)
#### 3.1 아키텍처 설계
   * 모듈 구조:
~~~
BMP_Processor
├─── File_Handler
│  ├─── Input_Validator
│  └─── Output_Generator
├─── Image_Processor
│  ├─── Color__Converter
│  ├─── Edge_Detector
│  └─── Memory__Manager
└─── Error_Handler
~~~

#### 3.2 상세 설계
 * 주요 상수: `IMAGE_WIDTH = 630`, `IMAGE_HEIGHT = 630`
 * 구조체 정의: `BMPFileHeader`, `BMPInfoHeader`, `RGB`
 * 주요 함수:
   * `main()` : 파일 입출력, 메모리 관리, 함수 호출 등 메인 프로세스 제어
   * `rgbToGrayscale()` : RGB-그레이스케일 변환
   * `findEdges()` : 소벨 필터 엣지 검출
   * `getPixel()` : 경계 검사를 포함한 안전한 픽셀 접근

#### 3.3 인터페이스 설계
 * 입력 인터페이스:
   * 파일명: `brainct_001.bmp`
   * 형식: 24비트 컬러 BMP, 630x630 픽셀
 * 출력 인터페이스:
   * `output_grayscale.bmp` (8비트 그레이스케일 BMP)
   * `output_edge.bmp` (8비트 엣지 검출 BMP)
   * `output_image.mem` (16진수 텍스트)

### 4. 소프트웨어 코드 표준 (SCS)
#### 4.1 코딩 규칙
 * 명명 규칙: PascalCase (구조체), camelCase (함수/변수), UPPER_CASE (상수)
 * 코드 스타일: 들여쓰기 4칸, K&R 중괄호 스타일, 한 줄당 하나의 명령문

### 5. 소프트웨어 검증 계획 (SVP)
#### 5.3 테스트 케이스
 * TC-001 : 정상 입력 처리
   * 유효한 630x630 24비트 BMP 파일(`brainct_001.bmp`) 입력
   * 예상 결과: 성공적인 변환 및 모든 출력 파일(그레이스케일, 엣지, MEM) 생성
 * TC-002 : 잘못된 파일 형식 또는 크기
   * 24비트가 아니거나 630x630이 아닌 파일 입력
   * 예상 결과: 오류 메시지 출력 및 프로그램 종료
 * TC-003 : 파일 부재
   * 입력 파일(`brainct_001.bmp`)이 없는 경우
   * 예상 결과: 파일 열기 실패 메시지 출력
 * TC-004 : 엣지 검출 결과 검증
   * 생성된 `output_edge.bmp` 파일을 육안으로 확인하여 엣지가 올바르게 표현되었는지 검증

### 7. 추적성 매트릭스

 | 요구사항 ID | 설계 요소 | 코드 구현 | 테스트 케이스 |
 |---|---|---|---|
 | HLR-001 | Input_Validator | `main()` 헤더 검증 | TC-001, TC-002 |
 | HLR-002 | Color_Converter | `rgbToGrayscale()` | TC-001 |
 | HLR-003 | Edge_Detector | `findEdges()`, `getPixel()` | TC-001, TC-004 |
 | HLR-004 | Output_Generator| `fwrite()`, `fprintf()` | TC-001 |
 | HLR-005 | Memory_Manager | `malloc()`, `free()` | TC-001 |
 | HLR-006 | Error_Handler | `if`, `printf` | TC-002, TC-003 |

### 8. 인증 결론
   * 본 BMP 이미지 처리 소프트웨어는 DO-178C DAL-D 수준의 요구사항을 충족하도록 설계 및 구현되었다.
   * 모든 필수 문서가 작성되었으며, 계획된 검증 활동을 통해 소프트웨어의 안전성과 신뢰성이 입증될 것이다.
