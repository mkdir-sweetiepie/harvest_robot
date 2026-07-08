# Crop Harvesting Robot — Chambit Design Capstone 2025

> 광운대 창빛설계학기 캡스톤 · 작물수확로봇 (3위). YOLOv12 + FoundationPose 6D + 6DOF 암.

## 구성

| 폴더 | 내용 |
|------|------|
| `control_tower/` | Qt GUI 통합관제 (비전+모터+경로) |
| `motion/` | 동작 제어 ROS2 패키지 |
| `path_planning_tsp/` | TSP 수확순서 최적화 (Held-Karp) + Qt5 3D 시각화 |

각 서브폴더의 원본 README에 상세 내용이 있습니다.

---
*여러 개로 흩어져 있던 저장소를 하나로 통합. 원본은 `_archive_superseded/`에 보관.*