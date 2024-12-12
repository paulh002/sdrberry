import{z as ve,r as g,c as d,b as ce,B as he,C as q,Z as ge,H as de,D as me,w as D,I as ye,ac as fe,ad as pe,Q as be,U as L,G as B,o as r,a as u,i as k,L as n,j as ke,l as x,m as f,p as H,q as S,u as h,M as Z,n as J,J as xe,f as we,P as z,k as O,e as w,h as A}from"./index-DIL_pLmJ.js";import{c as X}from"./useNestedProp-BxrcwmTZ-1Bk3HyKT.js";import{x as Ce}from"./Title-DyD-mBPN-CHqvPevn.js";import{C as $e}from"./Legend-BorSDXCj-Cb5_Z7Gi.js";import{r as Le,a as Be}from"./usePrinter-CLcxWv-4-CCDxi2Mm.js";import{p as Ae}from"./PackageVersion-mSsg8dpu-BxnQ4_xj.js";import{S as Pe}from"./PenAndPaper-Dk41ZrJ0-D5uht0-m.js";import Fe from"./vue-ui-skeleton-Dgzcn3FL-BNvPX0g2.js";import{s as Ie}from"./vue-data-ui-DF7u-eZV-hcmE5A1B.js";import"./Shape-DYFnvMNd-DLTmTYE_.js";const _e=["id"],He={key:1,ref:"noTitle",class:"vue-data-ui-no-title-space",style:"height:36px; width: 100%;background:transparent"},Se=["xmlns","viewBox"],ze={key:0},Oe=["x","y","height","width","fill","stroke"],We=["x","y","height","width","rx","fill","stroke","stroke-width"],Me=["x","y","height","width","fill","stroke","stroke-width"],Te=["x","y","font-size","font-weight","fill"],Ue=["x","y","height","width","rx","fill","stroke","stroke-width"],Re={key:3},Ve=["x","y","fill","font-size","font-weight"],je={key:4},Ne=["x1","x2","y1","y2","stroke"],Ee={key:5,class:"vue-data-ui-watermark"},Ge={class:"vue-ui-bullet-legend-item",dir:"auto"},qe={style:{"margin-right":"2px"}},De={key:7,ref:"source",dir:"auto"},Ze={__name:"vue-ui-bullet",props:{config:{type:Object,default(){return{}}},dataset:{type:Object,default(){return{}}}},setup(Y,{expose:Q}){const o=Y,{vue_ui_bullet:K}=ve(),P=g(null),ee=g(null),W=g(0),te=g(null),ae=g(0),m=d({get:()=>o.dataset.hasOwnProperty("value"),set:t=>t}),p=d(()=>o.dataset.segments?Array.isArray(o.dataset.segments)?o.dataset.segments.length?!0:(console.warn(`VueUiBullet: dataset segments is empty. Provide segments with this datastructure:

segments: [
    {
        name: string;
        from: number;
        to: number;
        color?: string;
    },
    {...}
]
        `),m.value=!1,!1):(console.warn(`VueUiBullet: dataset segments must be an array of objects with this datastructure:

segments: [
    {
        name: string;
        from: number;
        to: number;
        color?: string;
    },
    {...}
] 
        `),m.value=!1,!1):(console.warn(`VueUiBullet: dataset segments is empty. Provide segments with this datastructure:

segments: [
    {
        name: string;
        from: number;
        to: number;
        color?: string;
    },
    {...}
]
        `),m.value=!1,!1));ce(M);function M(){he(o.dataset)?q({componentName:"VueUiBullet",type:"dataset"}):p.value?o.dataset.segments.forEach((t,s)=>{ge({datasetObject:t,requiredAttributes:["name","from","to"]}).forEach(a=>{m.value=!1,q({componentName:"VueUiBullet segment",type:"datasetSerieAttribute",property:a,index:s})})}):m.value=!1,e.value.style.chart.animation.show&&R(o.dataset.value||0)}const F=g(me());function T(){const t=X({userConfig:o.config,defaultConfig:K});return t.theme?{...X({userConfig:de.vue_ui_bullet[t.theme]||o.config,defaultConfig:t})}:t}const e=d({get:()=>T(),set:t=>t});D(()=>o.config,t=>{e.value=T(),M(),W.value+=1},{deep:!0});const l=d(()=>{const t=e.value.style.chart.height,s=e.value.style.chart.width,a=e.value.style.chart.padding.left,y=s-e.value.style.chart.padding.right,$=e.value.style.chart.padding.top,c=t-e.value.style.chart.padding.bottom;return{height:t,width:s,left:a,right:y,top:$,bottom:c,chartWidth:y-a,chartHeight:c-$}}),le=d(()=>{if(!p.value)return[];const t=[];for(let s=0;s<o.dataset.segments.length;s+=1)t.push(ye(e.value.style.chart.segments.baseColor,s/o.dataset.segments.length));return t}),I=d(()=>p.value?{min:Math.min(...o.dataset.segments.map(t=>t.from)),max:Math.max(...o.dataset.segments.map(t=>t.to))}:{min:0,max:1}),i=g(se());D(()=>o.dataset,t=>{e.value.style.chart.animation.show?R(t.value||0):i.value=t.value||0},{deep:!0});function se(){return e.value.style.chart.animation.show?I.value.min:o.dataset.value||0}const U=g(null);function R(t){const s=Math.abs(t-i.value)/e.value.style.chart.animation.animationFrames;function a(){i.value<t?i.value=Math.min(i.value+s,t):i.value>t&&(i.value=Math.max(i.value-s,t)),i.value!==t&&(U.value=requestAnimationFrame(a))}a()}fe(()=>{cancelAnimationFrame(U.value)});const v=d(()=>{if(!p.value)return[];const t=pe(I.value.min,I.value.max,e.value.style.chart.segments.ticks.divisions),s=t.min>=0?0:Math.abs(t.min),a={x:l.value.left+(o.dataset.target+s)/(t.max+s)*l.value.chartWidth-e.value.style.chart.target.width/2},y={width:(i.value+s)/(t.max+s)*l.value.chartWidth},$=t.ticks.map(c=>({value:c,y:l.value.bottom+e.value.style.chart.segments.dataLabels.fontSize+3+e.value.style.chart.segments.dataLabels.offsetY,x:l.value.left+(c+s)/(t.max+s)*l.value.chartWidth}));return{scale:t,target:a,value:y,ticks:$,chunks:o.dataset.segments.map((c,ie)=>({...c,color:c.color?be(c.color):le.value[ie],x:l.value.left+l.value.chartWidth*((c.from+s)/(t.max+s)),y:l.value.top,height:l.value.chartHeight,width:l.value.chartWidth*(Math.abs(c.to-c.from)/(t.max+s))}))}}),V=d(()=>!v.value||!v.value.chunks||!v.value.chunks.length?[]:v.value.chunks.map(t=>{const s=L(e.value.style.chart.segments.dataLabels.formatter,t.from,B({p:e.value.style.chart.segments.dataLabels.prefix,v:t.from,s:e.value.style.chart.segments.dataLabels.suffix,r:e.value.style.chart.segments.dataLabels.rounding})),a=L(e.value.style.chart.segments.dataLabels.formatter,t.to,B({p:e.value.style.chart.segments.dataLabels.prefix,v:t.to,s:e.value.style.chart.segments.dataLabels.suffix,r:e.value.style.chart.segments.dataLabels.rounding}));return{...t,shape:"square",value:`${s} - ${a}`}})),re=d(()=>({cy:"bullet-div-legend",backgroundColor:"transparent",color:e.value.style.chart.legend.color,fontSize:e.value.style.chart.legend.fontSize,paddingBottom:6,fontWeight:e.value.style.chart.legend.bold?"bold":""})),{isPrinting:j,isImaging:N,generatePdf:E,generateImage:G}=Le({elementId:`bullet_${F.value}`,fileName:e.value.style.chart.title.text||"vue-ui-bullet"}),oe=d(()=>e.value.userOptions.show&&!e.value.style.chart.title.text),b=g(!1);function ue(t){b.value=t,ae.value+=1}function ne(){return v.value}const C=g(!1);function _(){C.value=!C.value}return Q({getData:ne,generatePdf:E,generateImage:G,toggleAnnotator:_}),(t,s)=>(r(),u("div",{ref_key:"bulletChart",ref:P,class:J(`vue-ui-bullet ${b.value?"vue-data-ui-wrapper-fullscreen":""}`),style:Z(`font-family:${e.value.style.fontFamily};width:100%;background:${e.value.style.chart.backgroundColor}`),id:`bullet_${F.value}`},[e.value.userOptions.buttons.annotator?(r(),k(Pe,{key:0,parent:P.value,backgroundColor:e.value.style.chart.backgroundColor,color:e.value.style.chart.color,active:C.value,onClose:_},null,8,["parent","backgroundColor","color","active"])):n("",!0),oe.value?(r(),u("div",He,null,512)):n("",!0),e.value.style.chart.title.text?(r(),u("div",{key:2,ref_key:"chartTitle",ref:ee,style:"width:100%;background:transparent;"},[(r(),k(Ce,{lineHeight:"1.3rem",key:`title_${W.value}`,config:{title:{cy:"bullet-div-title",...e.value.style.chart.title},subtitle:{cy:"bullet-div-subtitle",...e.value.style.chart.title.subtitle}}},null,8,["config"]))],512)):n("",!0),e.value.userOptions.show&&m.value?(r(),k(Be,{key:3,ref:"details",backgroundColor:e.value.style.chart.backgroundColor,color:e.value.style.chart.color,isPrinting:h(j),isImaging:h(N),uid:F.value,hasTooltip:!1,hasPdf:e.value.userOptions.buttons.pdf,hasImg:e.value.userOptions.buttons.img,hasXls:!1,hasTable:!1,hasLabel:!1,hasFullscreen:e.value.userOptions.buttons.fullscreen,isFullscreen:b.value,chartElement:P.value,position:e.value.userOptions.position,titles:{...e.value.userOptions.buttonTitles},hasAnnotator:e.value.userOptions.buttons.annotator,isAnnotation:C.value,onToggleFullscreen:ue,onGeneratePdf:h(E),onGenerateImage:h(G),onToggleAnnotator:_},ke({_:2},[t.$slots.optionPdf?{name:"optionPdf",fn:x(()=>[f(t.$slots,"optionPdf",{},void 0,!0)]),key:"0"}:void 0,t.$slots.optionImg?{name:"optionImg",fn:x(()=>[f(t.$slots,"optionImg",{},void 0,!0)]),key:"1"}:void 0,t.$slots.optionFullscreen?{name:"optionFullscreen",fn:x(({toggleFullscreen:a,isFullscreen:y})=>[f(t.$slots,"optionFullscreen",H(S({toggleFullscreen:a,isFullscreen:y})),void 0,!0)]),key:"2"}:void 0,t.$slots.optionAnnotator?{name:"optionAnnotator",fn:x(({toggleAnnotator:a,isAnnotator:y})=>[f(t.$slots,"optionAnnotator",H(S({toggleAnnotator:a,isAnnotator:y})),void 0,!0)]),key:"3"}:void 0]),1032,["backgroundColor","color","isPrinting","isImaging","uid","hasPdf","hasImg","hasFullscreen","isFullscreen","chartElement","position","titles","hasAnnotator","isAnnotation","onGeneratePdf","onGenerateImage"])):n("",!0),m.value?(r(),u("svg",{key:4,xmlns:h(xe),class:J({"vue-data-ui-fullscreen--on":b.value,"vue-data-ui-fulscreen--off":!b.value,"vue-ui-bullet-svg":!0}),viewBox:`0 0 ${l.value.width} ${l.value.height}`,style:Z(`width: 100%; overflow: visible; background:transparent;color:${e.value.style.chart.color}`)},[we(Ae),p.value?(r(),u("g",ze,[(r(!0),u(z,null,O(v.value.chunks,a=>(r(),u("rect",{x:a.x,y:a.y,height:a.height,width:a.width,fill:a.color,"stroke-width":1,stroke:e.value.style.chart.backgroundColor},null,8,Oe))),256)),e.value.style.chart.target.onTop?n("",!0):(r(),u("rect",{key:0,x:v.value.target.x,y:l.value.top+(l.value.chartHeight-l.value.chartHeight*e.value.style.chart.target.heightRatio)/2,height:l.value.chartHeight*e.value.style.chart.target.heightRatio,width:e.value.style.chart.target.width,rx:e.value.style.chart.target.rounded?e.value.style.chart.target.width/2:0,fill:e.value.style.chart.target.color,stroke:e.value.style.chart.target.stroke,"stroke-width":e.value.style.chart.target.strokeWidth},null,8,We)),w("rect",{x:l.value.left,y:l.value.top+(l.value.chartHeight-l.value.chartHeight*e.value.style.chart.valueBar.heightRatio)/2,height:l.value.chartHeight*e.value.style.chart.valueBar.heightRatio,width:v.value.value.width,fill:e.value.style.chart.valueBar.color,stroke:e.value.style.chart.valueBar.stroke,"stroke-width":e.value.style.chart.valueBar.strokeWidth},null,8,Me),e.value.style.chart.valueBar.label.show?(r(),u("text",{key:1,x:l.value.left+v.value.value.width,y:l.value.top-6+e.value.style.chart.valueBar.label.offsetY,"font-size":e.value.style.chart.valueBar.label.fontSize,"font-weight":e.value.style.chart.valueBar.label.bold?"bold":"normal",fill:e.value.style.chart.valueBar.label.color,"text-anchor":"middle"},A(h(L)(e.value.style.chart.segments.dataLabels.formatter,i.value,h(B)({p:e.value.style.chart.segments.dataLabels.prefix,v:i.value,s:e.value.style.chart.segments.dataLabels.suffix,r:e.value.style.chart.segments.dataLabels.rounding}))),9,Te)):n("",!0),e.value.style.chart.target.onTop?(r(),u("rect",{key:2,x:v.value.target.x,y:l.value.top+(l.value.chartHeight-l.value.chartHeight*e.value.style.chart.target.heightRatio)/2,height:l.value.chartHeight*e.value.style.chart.target.heightRatio,width:e.value.style.chart.target.width,rx:e.value.style.chart.target.rounded?e.value.style.chart.target.width/2:0,fill:e.value.style.chart.target.color,stroke:e.value.style.chart.target.stroke,"stroke-width":e.value.style.chart.target.strokeWidth},null,8,Ue)):n("",!0),e.value.style.chart.segments.dataLabels.show?(r(),u("g",Re,[(r(!0),u(z,null,O(v.value.ticks,a=>(r(),u("text",{x:a.x,y:a.y,"text-anchor":"middle",fill:e.value.style.chart.segments.dataLabels.color,"font-size":e.value.style.chart.segments.dataLabels.fontSize+"px","font-weight":e.value.style.chart.segments.dataLabels.bold?"bold":"normal"},A(h(L)(e.value.style.chart.segments.dataLabels.formatter,a.value,h(B)({p:e.value.style.chart.segments.dataLabels.prefix,v:a.value,s:e.value.style.chart.segments.dataLabels.suffix,r:e.value.style.chart.segments.dataLabels.rounding}))),9,Ve))),256))])):n("",!0),e.value.style.chart.segments.dataLabels.show&&e.value.style.chart.segments.ticks.show?(r(),u("g",je,[(r(!0),u(z,null,O(v.value.ticks,a=>(r(),u("line",{x1:a.x,x2:a.x,y1:l.value.bottom,y2:l.value.bottom+3,stroke:e.value.style.chart.segments.ticks.stroke,"stroke-width":1,"stroke-linecap":"round"},null,8,Ne))),256))])):n("",!0)])):n("",!0),f(t.$slots,"svg",{svg:l.value},void 0,!0)],14,Se)):n("",!0),t.$slots.watermark?(r(),u("div",Ee,[f(t.$slots,"watermark",H(S({isPrinting:h(j)||h(N)})),void 0,!0)])):n("",!0),m.value?n("",!0):(r(),k(Fe,{key:6,config:{type:"bullet",style:{backgroundColor:e.value.style.chart.backgroundColor,bullet:{color:"#CCCCCC"}}}},null,8,["config"])),w("div",{ref_key:"chartLegend",ref:te},[e.value.style.chart.legend.show?(r(),k($e,{key:0,clickable:!1,legendSet:V.value,config:re.value},{item:x(({legend:a})=>[w("div",Ge,[w("span",qe,A(a.name)+":",1),w("span",null,A(a.value),1)])]),_:1},8,["legendSet","config"])):n("",!0),f(t.$slots,"legend",{legend:V.value},void 0,!0)],512),t.$slots.source?(r(),u("div",De,[f(t.$slots,"source",{},void 0,!0)],512)):n("",!0)],14,_e))}},rt=Ie(Ze,[["__scopeId","data-v-cfe188c3"]]);export{rt as default};
