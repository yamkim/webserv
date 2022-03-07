# 구현할 기능 명세

## HTTP Server similar to nginx

- Static File Server
    - MIME 확인
    - HTTP Request / Response 확인
        - 응답 코드 및 헤더 확인
    - 잘못된 url 처리
    - 영속성 유지
    - 적절한 상태 코드 사용
- 설정에 따른 동작 확인
    - 디폴트 파일
    - 라우팅
    - 오토인덱스
    - 포트
    - 라우트 된 경로에 특정 메소드만 허용
    - 에러 페이지
    - 리다이렉트

## CGI

- 파일 업로드 및 다운로드 구현
- POST 테스트
- 캐시와 쿠키 확인

## 검증

- 브라우저 접속
- curl 또는 postman

## 부하 테스트

- ab나 siege 이용
    - Mac OS 기준 ab는 내장되어 있으며 siege는 별도 설치 필요