# Tension Up!

> 브릿지 2025-1 프로젝트 (2025.03 ~ 2025.08)

<p align="center">
  <img src="docs/images/swing.jpg" alt="케이블에 매달려 스윙하는 플레이 화면" width="720">
</p>

<p align="center">
  <a href="https://www.youtube.com/watch?v=-7HOen4yWeI">
    <img src="https://img.shields.io/badge/YouTube-플레이%20영상-FF0000?style=for-the-badge&logo=youtube&logoColor=white" alt="YouTube 플레이 영상">
  </a>
</p>

<br/>

## 소개

케이블을 걸어 정상까지 오르는 물리 기반 3D 항아리 게임입니다.
주인공 벤은 케이블과 증기 부스터만으로 정상까지 올라가야 합니다.

<br/>

## 주요 코드

| 파일 | 내용 |
|---|---|
| [CableActionComponent](Source/UncleWeb/Component/CableActionComponent.cpp) | 케이블 발사와 회수, 진자 운동 스윙 |
| [DynamicCamera](Source/UncleWeb/Component/DynamicCamera.cpp) | 속도에 따라 카메라 거리, FOV, 블러 조절 |
| [SteamComponent](Source/UncleWeb/Component/SteamComponent.cpp) | 증기 자원 소모와 회복 |
