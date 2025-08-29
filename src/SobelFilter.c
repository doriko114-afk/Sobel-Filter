#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h> // sqrt 함수를 위해 추가

// BMP 파일 헤더 구조체
#pragma pack(push, 1)
typedef struct {
    uint16_t type;        // 파일 형식 (BM)
    uint32_t size;        // 파일 크기
    uint16_t reserved1;   // 예약된 필드
    uint16_t reserved2;   // 예약된 필드
    uint32_t offset;      // 이미지 데이터 시작 위치
} BMPFileHeader;

typedef struct {
    uint32_t size;        // 헤더 크기
    int32_t width;        // 이미지 너비
    int32_t height;       // 이미지 높이
    uint16_t planes;      // 색상 평면 수
    uint16_t bitCount;    // 픽셀당 비트 수
    uint32_t compression; // 압축 형식
    uint32_t sizeImage;   // 이미지 데이터 크기
    int32_t xPelsPerMeter;// 수평 해상도
    int32_t yPelsPerMeter;// 수직 해상도
    uint32_t clrUsed;     // 사용된 색상 수
    uint32_t clrImportant;// 중요한 색상 수
} BMPInfoHeader;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} RGB;
#pragma pack(pop)

// =================================================================
// 소벨 필터 (Sobel Filter) 엣지 검출 관련 함수
// - sobelX, sobelY: 엣지를 검출하기 위한 행렬(커널)
// - getPixel: 이미지 경계를 벗어나지 않도록 안전하게 픽셀 값을 가져오는 함수
// - findEdges: getPixel과 소벨 커널을 이용해 실제 엣지 검출을 수행하는 메인 함수
// =================================================================
int sobelX[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};
int sobelY[3][3] = {
    {-1, -2, -1},
    { 0,  0,  0},
    { 1,  2,  1}
};

// 안전하게 픽셀 값 가져오기 (경계 처리)
unsigned char getPixel(unsigned char* image, int width, int height, int x, int y) {
    if (x < 0) x = 0;
    if (x >= width) x = width - 1;
    if (y < 0) y = 0;
    if (y >= height) y = height - 1;
    return image[y * width + x];
}

// 소벨 필터로 엣지 찾기
void findEdges(unsigned char* input, unsigned char* output, int width, int height) {
    printf("엣지 검출 중...\n");
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int gx = 0, gy = 0;
            // 주변 9개 픽셀에 필터 적용
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    unsigned char pixel = getPixel(input, width, height, x + j, y + i);
                    gx += pixel * sobelX[i + 1][j + 1];
                    gy += pixel * sobelY[i + 1][j + 1];
                }
            }
            // 엣지 강도 계산
            int edge = (int)sqrt((double)gx * gx + (double)gy * gy);
            if (edge > 255) edge = 255;
            output[y * width + x] = (uint8_t)edge;
        }
    }
}

// RGB를 그레이스케일로 변환하는 함수
uint8_t rgbToGrayscale(RGB pixel) {
    // 표준 luminance 공식: Y = 0.299*R + 0.587*G + 0.114*B
    return (uint8_t)(0.299 * pixel.red + 0.587 * pixel.green + 0.114 * pixel.blue);
}

int main() {
    // 1. 파일명 변수 선언
    const char* inputFile = "goldmine.bmp";
    const char* outputFile = "output_grayscale.bmp";
    const char* edgeFile = "output_edge.bmp";        // 추가: 엣지 검출 결과를 저장할 파일명
    const char* memFile = "output_image.mem";
    const int IMAGE_WIDTH = 630;
    const int IMAGE_HEIGHT = 630;

    // 2. 파일 포인터 변수 선언
    FILE* inFile = NULL;
    FILE* outFile = NULL;
    FILE* edgeOutFile = NULL;                        // 추가: 엣지 검출 BMP 파일을 위한 파일 포인터
    FILE* memOutFile = NULL;

    // 입력 파일 열기
    if (fopen_s(&inFile, inputFile, "rb") != 0 || inFile == NULL) {
        printf("입력 파일을 열 수 없습니다: %s\n", inputFile);
        return 1;
    }

    // BMP 헤더 읽기 및 검증
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;
    fread(&fileHeader, sizeof(BMPFileHeader), 1, inFile);
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, inFile);

    if (fileHeader.type != 0x4D42 || infoHeader.bitCount != 24 || infoHeader.width != IMAGE_WIDTH || infoHeader.height != IMAGE_HEIGHT) {
        printf("지원하지 않는 BMP 파일 형식입니다.\n");
        fclose(inFile);
        return 1;
    }

    printf("입력 파일 정보:\n- 크기: %dx%d\n- 비트 수: %d\n", infoHeader.width, infoHeader.height, infoHeader.bitCount);

    // 원본 이미지 데이터 읽기
    int padding = (4 - (IMAGE_WIDTH * 3) % 4) % 4;
    RGB* imageData = (RGB*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(RGB));
    if (imageData == NULL) { printf("메모리 할당 실패\n"); fclose(inFile); return 1; }
    fseek(inFile, fileHeader.offset, SEEK_SET);
    for (int y = 0; y < IMAGE_HEIGHT; y++) {
        fread(&imageData[y * IMAGE_WIDTH], sizeof(RGB), IMAGE_WIDTH, inFile);
        fseek(inFile, padding, SEEK_CUR);
    }
    fclose(inFile);

    // 그레이스케일 변환
    uint8_t* grayscaleData = (uint8_t*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT);
    if (grayscaleData == NULL) { printf("메모리 할당 실패\n"); free(imageData); return 1; }
    for (int i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++) {
        grayscaleData[i] = rgbToGrayscale(imageData[i]);
    }

    // 3. 엣지 검출 수행
    // 그레이스케일로 변환된 이미지를 기반으로 엣지를 검출하기 위해
    // 결과를 저장할 메모리를 할당하고, findEdges 함수를 호출합니다.
    uint8_t* edgeData = (uint8_t*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT);
    if (edgeData == NULL) {
        printf("엣지 데이터 메모리 할당 실패\n");
        free(imageData); free(grayscaleData);
        return 1;
    }
    findEdges(grayscaleData, edgeData, IMAGE_WIDTH, IMAGE_HEIGHT);

    // 그레이스케일 BMP 파일 저장
    int newPadding = (4 - (IMAGE_WIDTH % 4)) % 4;
    BMPFileHeader newFileHeader = fileHeader;
    BMPInfoHeader newInfoHeader = infoHeader;
    newFileHeader.offset = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + 256 * 4;
    newFileHeader.size = newFileHeader.offset + (IMAGE_WIDTH + newPadding) * IMAGE_HEIGHT;
    newInfoHeader.bitCount = 8;
    newInfoHeader.sizeImage = (IMAGE_WIDTH + newPadding) * IMAGE_HEIGHT;
    newInfoHeader.clrUsed = 256;
    newInfoHeader.clrImportant = 256;

    uint8_t palette[256 * 4];
    for (int i = 0; i < 256; i++) { palette[i * 4] = palette[i * 4 + 1] = palette[i * 4 + 2] = i; palette[i * 4 + 3] = 0; }

    if (fopen_s(&outFile, outputFile, "wb") != 0 || outFile == NULL) { printf("파일 생성 실패: %s\n", outputFile); /* ... */ return 1; }
    fwrite(&newFileHeader, sizeof(BMPFileHeader), 1, outFile);
    fwrite(&newInfoHeader, sizeof(BMPInfoHeader), 1, outFile);
    fwrite(palette, 1, 1024, outFile);
    for (int y = IMAGE_HEIGHT - 1; y >= 0; y--) {
        fwrite(&grayscaleData[y * IMAGE_WIDTH], 1, IMAGE_WIDTH, outFile);
        if (newPadding > 0) { uint8_t p[3] = { 0 }; fwrite(p, 1, newPadding, outFile); }
    }
    fclose(outFile);

    // 4. 엣지 검출 결과를 별도의 BMP 파일로 저장
    // 엣지 검출 결과를 시각적으로 확인하기 위해 그레이스케일 BMP와
    // 동일한 헤더 및 팔레트 정보를 사용하여 새로운 BMP 파일을 생성합니다.
    if (fopen_s(&edgeOutFile, edgeFile, "wb") != 0 || edgeOutFile == NULL) { printf("파일 생성 실    패: %s\n", edgeFile); /* ... */ return 1; }
    fwrite(&newFileHeader, sizeof(BMPFileHeader), 1, edgeOutFile);
    fwrite(&newInfoHeader, sizeof(BMPInfoHeader), 1, edgeOutFile);
    fwrite(palette, 1, 1024, edgeOutFile);
    for (int y = 0; y < IMAGE_HEIGHT; y++) 
    {
        fwrite(&edgeData[y * IMAGE_WIDTH], 1, IMAGE_WIDTH, edgeOutFile);
        if (newPadding > 0) { uint8_t p[3] = { 0 }; fwrite(p, 1, newPadding, edgeOutFile); }
    }
    fclose(edgeOutFile);

    // MEM 파일 만들기 (그레이스케일 기준)
    if (fopen_s(&memOutFile, memFile, "w") != 0 || memOutFile == NULL) { printf("파일 생성 실패: %s\n", memFile); /* ... */ return 1; }
    for (int i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++) {
        fprintf(memOutFile, "%02X\n", grayscaleData[i]);
    }
    fclose(memOutFile);

    // 6. 모든 동적 할당 메모리 해제
    free(imageData);
    free(grayscaleData);
    free(edgeData); // 추가: 엣지 데이터에 할당된 메모리 해제

    // 7. 최종 완료 메시지 출력
    printf("\n모든 변환 완료!\n");
    printf("그레이스케일 BMP 파일: %s\n", outputFile);
    printf("엣지 검출 BMP 파일: %s\n", edgeFile);        // 추가: 엣지 파일 생성 완료 메시지
    printf("그레이스케일 MEM 파일: %s\n", memFile);

    return 0;
}


