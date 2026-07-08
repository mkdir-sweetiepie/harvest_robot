# 수확 순서 정보
std_msgs/Header header
uint32[] crop_ids       # 수확할 작물 ID 순서
DetectedCrop[] objects  # 작물 객체 정보
uint32 total_objects    # 총 작물 수