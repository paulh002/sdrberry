import{c as b}from"./useNestedProp-BxrcwmTZ-CudEJDco.js";import{z as E,r as n,D as z,c as C,b as F,K as L,ac as _,o as l,a as s,e as a,S as B,T as I,u as p,O as $,L as i,n as x,h as g,M as D,J as O}from"./index-UNQvOp-_.js";import{s as T}from"./vue-data-ui-DF7u-eZV-Cm3hFNaA.js";const w=o=>(B("data-v-6a1383ea"),o=o(),I(),o),W=["xmlns","height","width"],G={id:"follower",fy:"30%",fx:"30%"},M=["stop-color","stop-opacity"],N=w(()=>a("stop",{offset:"95%","stop-color":"transparent"},null,-1)),A=["r","fill","stroke","stroke-width","stroke-dasharray"],J=["r","fill"],V={key:2,class:"wave"},X=["id"],Y=["id"],j=w(()=>a("feColorMatrix",{type:"saturate",values:"0"},null,-1)),K=["filter","stroke"],U={key:3,class:"crosshair"},q=["x1","stroke","stroke-width","stroke-dasharray"],H=["x1","stroke","stroke-width","stroke-dasharray"],P=["y1","stroke","stroke-width","stroke-dasharray"],Q=["y1","stroke","stroke-width","stroke-dasharray"],Z=["cy","r","fill"],ee=["cy","r","fill"],re=["cx","r","fill"],ae=["cx","r","fill"],te={key:5,class:"coordinates"},le=["x","y","font-size","fill"],se=["transform"],ie=["font-size","fill"],oe={__name:"vue-ui-cursor",props:{config:{type:Object,default:{}}},setup(o){const S=o,{vue_ui_cursor:R}=E(),v=n(z()),e=C(()=>b({userConfig:S.config,defaultConfig:R})),t=n({x:-100,y:-100});function f({clientX:r,clientY:m,...ce}){t.value.x=r-e.value.centerCircleRadius,t.value.y=m-e.value.centerCircleRadius}function h(r){t.value.x=r.targetTouches[0].clientX-e.value.centerCircleRadius,t.value.y=r.targetTouches[0].clientY-e.value.centerCircleRadius}const y=n(!0);function u(r){y.value=r}const c=n(!1),d=n(null);function k(){c.value=!1,d.value&&clearTimeout(d.value),L(()=>{c.value=!0,d.value=setTimeout(()=>{c.value=!1},1e3)})}return F(()=>{const r=e.value.parentId?document.getElementById(e.value.parentId):document.getElementsByTagName("div")[0];r.addEventListener("mousemove",f),r.addEventListener("touchmove",h),r.addEventListener("mouseleave",()=>u(!1)),r.addEventListener("mouseenter",()=>u(!0)),r.addEventListener("click",k)}),_(()=>{const r=e.value.parentId?document.getElementById(e.value.parentId):document.getElementsByTagName("div")[0];r.removeEventListener("mousemove",f),r.removeEventListener("touchmove",h),r.removeEventListener("mouseleave",()=>u(!1)),r.removeEventListener("mouseenter",()=>u(!0)),r.removeEventListener("click",k)}),C(()=>e.value.centerCircleRadius/2.5),(r,m)=>y.value?(l(),s("svg",{key:0,"data-html2canvas-ignore":"",xmlns:p(O),style:D(`z-index: 2147483647; overflow: visible; pointer-events: none;background: transparent; position:fixed; top:${t.value.y}px; left:${t.value.x}px;`),viewBox:"0 0 100 100",height:e.value.centerCircleRadius*2,width:e.value.centerCircleRadius*2},[a("defs",null,[a("radialGradient",G,[a("stop",{offset:"10%","stop-color":e.value.bubbleEffectColor,"stop-opacity":e.value.bubbleEffectOpacity},null,8,M),N])]),e.value.showCenterCircle?(l(),s("circle",{key:0,cx:50,cy:50,r:e.value.centerCircleRadius,fill:p($)(e.value.centerCircleColor,e.value.centerCircleOpacity*100),stroke:e.value.centerCircleStroke,"stroke-width":e.value.centerCircleStrokeWidth,"stroke-dasharray":e.value.centerCircleDasharray},null,8,A)):i("",!0),e.value.bubbleEffect?(l(),s("circle",{key:1,cx:50,cy:50,r:e.value.centerCircleRadius,fill:"url(#follower)",stroke:"none"},null,8,J)):i("",!0),e.value.useWaveOnClick?(l(),s("g",V,[a("defs",null,[a("filter",{id:`blur_${v.value}`,x:"-50%",y:"-50%",width:"200%",height:"200%"},[a("feGaussianBlur",{in:"SourceGraphic",stdDeviation:4,id:`blur_std_${v.value}`},null,8,Y),j],8,X)]),c.value?(l(),s("circle",{key:0,class:x({"circle-wave":c.value}),cx:50,cy:50,r:50,filter:`url(#blur_${v.value})`,stroke:e.value.centerCircleStroke,fill:"none","stroke-width":"3"},null,10,K)):i("",!0)])):i("",!0),e.value.showCrosshair?(l(),s("g",U,[a("line",{x1:-e.value.centerCircleRadius+50,x2:-5e3,y1:50,y2:50,stroke:e.value.crosshairStroke,"stroke-width":e.value.crosshairStrokeWidth,"stroke-dasharray":e.value.crosshairDasharray,"stroke-linecap":"round"},null,8,q),a("line",{x1:50+e.value.centerCircleRadius,x2:5e3,y1:50,y2:50,stroke:e.value.crosshairStroke,"stroke-width":e.value.crosshairStrokeWidth,"stroke-dasharray":e.value.crosshairDasharray,"stroke-linecap":"round"},null,8,H),a("line",{x1:50,x2:50,y1:-e.value.centerCircleRadius+50,y2:-5e3,stroke:e.value.crosshairStroke,"stroke-width":e.value.crosshairStrokeWidth,"stroke-dasharray":e.value.crosshairDasharray,"stroke-linecap":"round"},null,8,P),a("line",{x1:50,x2:50,y1:e.value.centerCircleRadius+50,y2:5e3,stroke:e.value.crosshairStroke,"stroke-width":e.value.crosshairStrokeWidth,"stroke-dasharray":e.value.crosshairDasharray,"stroke-linecap":"round"},null,8,Q)])):i("",!0),e.value.showIntersectCircles?(l(),s("g",{key:4,class:x({"rotating-circles":e.value.isLoading})},[a("circle",{cx:50,cy:e.value.centerCircleRadius+50,r:e.value.intersectCirclesRadius,fill:e.value.intersectCirclesFill},null,8,Z),a("circle",{cx:50,cy:-e.value.centerCircleRadius+50,r:e.value.intersectCirclesRadius,fill:e.value.intersectCirclesFill},null,8,ee),a("circle",{cx:-e.value.centerCircleRadius+50,cy:50,r:e.value.intersectCirclesRadius,fill:e.value.intersectCirclesFill},null,8,re),a("circle",{cx:e.value.centerCircleRadius+50,cy:50,r:e.value.intersectCirclesRadius,fill:e.value.intersectCirclesFill},null,8,ae)],2)):i("",!0),e.value.showCoordinates?(l(),s("g",te,[a("text",{"text-anchor":"end",x:-e.value.centerCircleRadius+50-e.value.coordinatesFontSize/2+e.value.coordinatesOffset,y:50-e.value.coordinatesFontSize/2+e.value.coordinatesOffset,"font-size":e.value.coordinatesFontSize,fill:e.value.coordinatesColor,style:{"font-variant-numeric":"tabular-nums"},"font-family":"Arial"},g(t.value.x.toFixed(0)),9,le),a("g",{transform:`translate(${50-e.value.coordinatesFontSize/2+e.value.coordinatesOffset}, ${-e.value.centerCircleRadius+50-e.value.coordinatesFontSize/2+e.value.coordinatesOffset})`},[a("text",{"text-anchor":"start","font-size":e.value.coordinatesFontSize,fill:e.value.coordinatesColor,style:{"font-variant-numeric":"tabular-nums"},transform:"rotate(-90)","font-family":"Arial"},g(t.value.y.toFixed(0)),9,ie)],8,se)])):i("",!0)],12,W)):i("",!0)}},de=T(oe,[["__scopeId","data-v-6a1383ea"]]);export{de as default};