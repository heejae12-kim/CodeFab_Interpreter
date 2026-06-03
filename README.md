# Agent 팀
: 단순히 지시받은 코드만 작성하는 것을 넘어, 스스로 생각하고, 동료와 협력하며, 최선의 코드를 자율적으로 찾아가는 주도적인 에이전트(Agent)가 되자.
---

## Ground rule
1) 코드리뷰 시간은 오전 11시에 20분, 오후에는 4:00에 30분
2) Request change에 대해서는 코드리뷰 시간에 상관없이 빠르게 리뷰한다.
3) 퇴근 시간은 17:00으로 한다. 
4) PR 생성 시 Approver는 2명 이상.
5) Merge된 feature branch는 PR 생성자가 삭제한다.
6) [Feature] Create test cases for kiwer , PR 생성시 Title 준수하기
7) PR단위는 100 line이하 지향하자.
8) Build pass / Unit Test 확인 후 PR 생성
---

## 코드리뷰 전략
1) Merge된 코드에 대해서는 과거로 돌아가는 rebase하지 않는다.
2) PR comment시 전체 변경 사항을 체크하고 칭찬 + 리뷰내용 + 요청사항 입력하기.
3) 의미있는 단위로 Commit 하기, 
 - 첫 개발단계에서는 class 단위로 commit 지향
 - Interface와 같은 공통 변경점은 회의 후 결정
4) Class 작성 시 
 - Header 파일에 define
 - Cpp파일에서 implementation 
---
