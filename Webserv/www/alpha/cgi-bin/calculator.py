#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# CGI: 즉시 계산 UI 렌더링 (연산은 클라이언트 JS에서 처리)

import sys

def main():
    print("Content-Type: text/html; charset=utf-8")
    print()
    print(r"""<!DOCTYPE html>
<html lang="ko">
<head>
  <meta charset="utf-8">
  <title>계산기</title>
  <link rel="icon" href="/www/sooslee_icon.ico">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    :root { font-family: system-ui, -apple-system, Segoe UI, Roboto, Arial, sans-serif; }
    body { margin: 40px; }
    .calc { max-width: 420px; margin: auto; padding: 20px; border: 1px solid #ddd; border-radius: 14px; }
    .screen { background:#111; color:#0f0; padding:16px; border-radius:10px; font-size:1.4rem; line-height:1.4; min-height:64px; }
    .result { font-weight:700; font-size:1.6rem; }
    .history { color:#a5ffa5; font-size:0.9rem; margin-top:6px; word-break:break-all; white-space:pre-wrap; }
    .row { display:flex; gap:8px; margin-top:12px; }
    input[type="text"] { flex:1; padding:12px; border:1px solid #ccc; border-radius:10px; font-size:1.1rem; }
    button { padding:12px; border:0; border-radius:10px; cursor:pointer; font-size:1.1rem; }
    .op { background:#4CAF50; color:#fff; flex:1; }
    .op:hover { filter:brightness(0.95); }
    .equal { background:#0b57d0; color:#fff; flex:1; }
    .clear { background:#666; color:#fff; width:100%; }
    .warn { color:#b00020; margin-top:8px; min-height:1.2em; }
    .tips { font-size:0.9rem; color:#555; margin-top:8px; }
    .grid { display:grid; grid-template-columns:repeat(4, 1fr); gap:8px; margin-top:12px; }
    .digit { padding:12px; background:#f3f3f3; border:1px solid #ddd; border-radius:10px; font-size:1.1rem; cursor:pointer; }
    .digit:hover { filter:brightness(0.97); }
    .full { grid-column:1 / -1; }
  </style>
</head>
<body>
  <div class="calc">
    <div class="screen" aria-live="polite">
      <div class="result" id="result">0</div>
      <div class="history" id="history"></div>
    </div>

    <div class="row">
      <input type="text" id="input" inputmode="decimal" placeholder="숫자를 입력하고 연산 버튼을 누르세요 (Enter=)">
    </div>

    <div class="row">
      <button class="op" data-op="+">＋</button>
      <button class="op" data-op="-">−</button>
      <button class="op" data-op="*">×</button>
      <button class="op" data-op="/">÷</button>
    </div>

    <div class="row">
      <button class="equal" id="eq">=</button>
    </div>

    <div class="grid">
      <button class="digit">7</button>
      <button class="digit">8</button>
      <button class="digit">9</button>
      <button class="digit">C</button>

      <button class="digit">4</button>
      <button class="digit">5</button>
      <button class="digit">6</button>
      <button class="digit">⌫</button>

      <button class="digit">1</button>
      <button class="digit">2</button>
      <button class="digit">3</button>
      <button class="digit">.</button>

      <button class="digit full">0</button>
    </div>

    <div class="row">
      <button class="clear" id="clear">초기화</button>
    </div>

    <div class="warn" id="warn"></div>
    <div class="tips">단축키: +, -, *, /, Enter(=), C(초기화), Backspace(지우기)</div>
  </div>

  <script>
  (function(){
    const $ = (id)=>document.getElementById(id);
    const resultEl = $("result");
    const historyEl = $("history");
    const inputEl = $("input");
    const warnEl = $("warn");

    let acc = 0;            // 누적 값
    let pendingOp = null;   // 마지막으로 눌린 연산자 (다음 값 입력 후 Enter(=)로 적용)
    let lastApplied = null; // 마지막으로 적용된 연산자 (수식 표시용)
    let pristine = true;    // 시작 상태

    function showWarn(msg){ warnEl.textContent = msg || ""; }

    function parseNum(s){
      if (!s || !s.trim()) return null;
      // 숫자/소수점만 허용 (선행 +/- 허용)
      if (!/^[+-]?\d*(\.\d+)?$/.test(s.trim())) return null;
      const n = Number(s);
      if (!Number.isFinite(n)) return null;
      return n;
    }

    function updateScreen(){
      resultEl.textContent = format(acc);
      historyEl.textContent = buildHistory();
    }

    function format(n){
      // 보기 좋게: 정수면 소수점 제거, 아니면 최대 12자리 표시
      if (Number.isInteger(n)) return String(n);
      return Number(n.toFixed(12)).toString();
    }

    function buildHistory(){
      // 간단: 마지막 적용 연산만 표기 (필요하면 로그 누적 가능)
      return lastApplied ? ("최근 적용: " + lastApplied) : "";
    }

    function apply(op, operandStr){
      showWarn("");
      const v = parseNum(operandStr);
      if (v === null){
        showWarn("유효한 숫자를 입력하세요.");
        return;
      }
      if (op === "/" && v === 0){
        showWarn("0으로 나눌 수 없습니다.");
        return;
      }
      if (pristine){
        // 첫 입력: 누적값을 입력값으로 시작
        acc = v;
        lastApplied = "시작값 = " + format(acc);
        pristine = false;
      } else {
        // 누적 적용
        if (op === "+") acc += v;
        else if (op === "-") acc -= v;
        else if (op === "*") acc *= v;
        else if (op === "/") acc /= v;
        lastApplied = formatPrev(format(acc), op, v); // 참고용 문자열 (간단히 대체)
      }
      inputEl.value = "";
      updateScreen();
    }

    function formatPrev(current, op, v){
      return op + " " + format(v) + " → " + current;
    }

    // 연산 버튼을 누르면 “입력칸 값”이 즉시 누적에 반영되고,
    // 그 연산자를 다음 단계의 기본 연산(pendingOp)으로 설정합니다.
    document.querySelectorAll(".op").forEach(btn=>{
      btn.addEventListener("click", ()=>{
        const op = btn.getAttribute("data-op");
        const val = inputEl.value;
        // 입력이 비어있고 아직 시작 안했으면 0 기준으로 시작
        if (pristine && (!val || !val.trim())){
          acc = 0;
          pristine = false;
          lastApplied = "시작값 = 0";
          updateScreen();
        } else {
          apply(op, val || "0");
        }
        pendingOp = op;
        inputEl.focus();
      });
    });

    // = 버튼: pendingOp가 있으면 그 연산으로 적용. 없으면 마지막 op로 다시 적용하지 않음.
    $("eq").addEventListener("click", ()=>{
      if (!pendingOp){
        // pending 없으면 입력값을 새 시작값으로 덮어쓰는 느낌으로 처리
        const v = parseNum(inputEl.value);
        if (v === null){
          showWarn("유효한 숫자를 입력하세요.");
          return;
        }
        acc = v;
        lastApplied = "시작값 = " + format(acc);
        pristine = false;
        inputEl.value = "";
        updateScreen();
        return;
      }
      apply(pendingOp, inputEl.value || "0");
      // = 이후에도 같은 op를 계속 사용할 수 있게 유지 (일반 계산기 느낌)
      inputEl.focus();
    });

    // 숫자 패드
    document.querySelectorAll(".digit").forEach(btn=>{
      btn.addEventListener("click", ()=>{
        const t = btn.textContent;
        if (t === "C"){ clearAll(); return; }
        if (t === "⌫"){ backspace(); return; }
        if (t === "."){
          if (!inputEl.value.includes(".")) inputEl.value += ".";
          inputEl.focus();
          return;
        }
        // 숫자
        inputEl.value += t;
        inputEl.focus();
      });
    });

    // 초기화
    function clearAll(){
      acc = 0;
      pendingOp = null;
      lastApplied = null;
      pristine = true;
      inputEl.value = "";
      showWarn("");
      updateScreen();
    }
    $("clear").addEventListener("click", clearAll);

    function backspace(){
      inputEl.value = inputEl.value.slice(0, -1);
    }

    // 단축키
    document.addEventListener("keydown", (e)=>{
      if (e.key === "Enter"){ e.preventDefault(); $("eq").click(); }
      else if (e.key === "+"){ e.preventDefault(); document.querySelector('[data-op="+"]').click(); }
      else if (e.key === "-"){ e.preventDefault(); document.querySelector('[data-op="-"]').click(); }
      else if (e.key === "*"){ e.preventDefault(); document.querySelector('[data-op="*"]').click(); }
      else if (e.key === "/"){ e.preventDefault(); document.querySelector('[data-op="/"]').click(); }
      else if (e.key === "Backspace"){ /* 기본 동작으로 입력칸 지우기 */ }
      else if (e.key.toLowerCase() === "c"){ clearAll(); }
    });

    // 최초 표시
    updateScreen();
    inputEl.focus();
  })();
  </script>
</body>
</html>
""")

if __name__ == "__main__":
    main()
