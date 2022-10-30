# Workenv

DG 는 많은 OS 의존적인 라이브러리들을 포함하고 있기 떄문에 이미 다른 버전의 의존성들이
OS 레벨에 설치가 되어 있을 경우 빌드까지 많은 시간이 소요될 수 있습니다.
그래서 DG 소스를 독립된 도커 컨테이너 환경에서 빌드하고 개발을 진행하기 위해 구성된 설정입니다.

## Files

개발환경을 빌드하고 개발을 진행하기 위한 파일들은 `${PROJECT_DIR}/workenv` 내에 포함되어 있습니다.

## Environment Image Build

본 명령어는 호스트머신에서 실행합니다. 최초 개발환경 이미지를 빌드하기 위해 아래의 멍령어를 사용합니다.

```shell
$ docker build -f workenv/Dockerfile.workenv -t dg:{$your-tag} .
```

## Run Environment

본 명령어는 호스트머신에서 실행합니다. 빌드된 이미지를 실행하기 위해 아래의 명령어를 사용합니다.
여기서 참고해야할 것은 `.env` 호스트머신에서 개발을 진행 할 수 있게 호스트 머신에 존재하는 소스코드 볼륨을
이미지에 마운트한다는 것입니다. 호스트머신의 소스코드 경로는 ${} 
그외 다른 관련 설정은 `${PROJECT_DIR}/workenv/.env` 에서 확인할 수 있습니다.

```shell
$ docker-compose --env-file ./workenv/.env -f ./workenv/docker-compose.yml up
```

## Build Source Code

마운트된 소스코드를 컨테이너내에서 빌드하기 위해 사용하는 명령어입니다.
그러므로 본 명령어는 컨테이너 내에서 실행되어야 합니다.

```shell
$ /workenv/build.sh
```