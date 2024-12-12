import{z as Je,c,r as n,D as Ke,R as Qe,A as S,H as Ze,w as Le,B as el,C as se,Z as ll,b as al,ac as tl,E as ol,Q as ul,U as W,G as x,K as sl,V as rl,N as il,o as s,a as i,i as F,L as p,j as nl,l as f,m as g,p as E,q as G,u as h,M as Z,n as Ne,J as vl,f as Pe,e as m,P as _,k as H,O as cl,a1 as dl,S as hl,T as yl,h as z,am as pl,ag as gl,a2 as fl,a3 as bl,g as Se}from"./index-DIL_pLmJ.js";import{O as ml,k as wl}from"./useResponsive-CoxXLe23-CoJWR8uA.js";import{x as xl}from"./Title-DyD-mBPN-CHqvPevn.js";import{r as kl,a as $l}from"./usePrinter-CLcxWv-4-CCDxi2Mm.js";import{E as Cl}from"./Tooltip-qod9o4Vz-Dsvb9C65.js";import{W as Il}from"./DataTable-D8fcY_sJ-By-GDpuN.js";import{C as Tl}from"./Legend-BorSDXCj-Cb5_Z7Gi.js";import _l from"./vue-ui-skeleton-Dgzcn3FL-BNvPX0g2.js";import zl from"./vue-ui-accordion-m8rut1bP-DBR24duA.js";import{c as Fe}from"./useNestedProp-BxrcwmTZ-1Bk3HyKT.js";import{p as Al}from"./PackageVersion-mSsg8dpu-BxnQ4_xj.js";import{S as Bl}from"./PenAndPaper-Dk41ZrJ0-D5uht0-m.js";import{s as Ll}from"./vue-data-ui-DF7u-eZV-hcmE5A1B.js";import"./Shape-DYFnvMNd-DLTmTYE_.js";const Oe=q=>(hl("data-v-b51c7605"),q=q(),yl(),q),Nl=["id"],Pl=["xmlns","viewBox"],Sl=["id"],Fl=["stop-color"],Ol=["stop-color"],Vl=["id"],Ml=Oe(()=>m("feGaussianBlur",{in:"SourceGraphic",stdDeviation:2},null,-1)),Wl=Oe(()=>m("feColorMatrix",{type:"saturate",values:"0"},null,-1)),El=[Ml,Wl],Gl=["x","y","height","width"],Hl=["height","width"],ql=["rx","x","y","height","width","stroke","stroke-width","filter"],jl=["rx","x","y","height","width","fill","stroke","stroke-width","filter"],Rl=["x","y","height","width","filter"],Dl={key:0},Ul={key:1},Xl={key:2},Yl={key:3},Jl=["onMouseover","x","y","height","width"],Kl={key:5,class:"vue-data-ui-watermark"},Ql=["onClick"],Zl={key:0},ea={key:1},la=["innerHTML"],aa={__name:"vue-ui-waffle",props:{config:{type:Object,default(){return{}}},dataset:{type:Array,default(){return[]}}},emits:["selectLegend"],setup(q,{expose:Ve,emit:Me}){const b=q,{vue_ui_waffle:We}=Je(),X=c(()=>!!b.dataset&&b.dataset.length),A=n(Ke()),Ee=n(null),ee=n(!1),le=n(""),O=n(null),re=n(0),V=n(null),ie=n(null),ne=n(null),ve=n(null),ce=n(null),de=n(0),he=n(0),ye=n(0),e=c({get:()=>pe(),set:l=>l});function pe(){const l=Fe({userConfig:b.config,defaultConfig:We});return l.theme?{...Fe({userConfig:Ze.vue_ui_waffle[l.theme]||b.config,defaultConfig:l}),customPalette:Qe[l.theme]||S}:l}Le(()=>b.config,l=>{e.value=pe(),ge(),de.value+=1,he.value+=1,ye.value+=1},{deep:!0});const Y=n(null);function ge(){if(el(b.dataset)?se({componentName:"VueUiWaffle",type:"dataset"}):b.dataset.forEach((l,o)=>{ll({datasetObject:l,requiredAttributes:["name","values"]}).forEach(a=>{se({componentName:"VueUiWaffle",type:"datasetSerieAttribute",property:a,index:o})})}),e.value.responsive){const l=ml(()=>{const{width:o,height:a}=wl({chart:V.value,title:e.value.style.chart.title.text?ie.value:null,legend:e.value.style.chart.legend.show?ne.value:null,source:ve.value,noTitle:ce.value});B.value.width=o,B.value.height=a,k.value.width=o,k.value.height=a});Y.value=new ResizeObserver(l),Y.value.observe(V.value.parentNode)}}al(()=>{ge()}),tl(()=>{Y.value&&Y.value.disconnect()});const{isPrinting:fe,isImaging:be,generatePdf:me,generateImage:we}=kl({elementId:`vue-ui-waffle_${A.value}`,fileName:e.value.style.chart.title.text||"vue-ui-waffle"}),Ge=c(()=>e.value.userOptions.show&&!e.value.style.chart.title.text),xe=c(()=>ol(e.value.customPalette)),I=n({showTable:e.value.table.show,showTooltip:e.value.style.chart.tooltip.show}),B=n({height:512,width:512}),k=n({top:0,left:0,height:512,width:512}),L=c(()=>(k.value.width-e.value.style.chart.layout.grid.size*e.value.style.chart.layout.grid.spaceBetween)/e.value.style.chart.layout.grid.size),N=c(()=>(k.value.height-e.value.style.chart.layout.grid.size*e.value.style.chart.layout.grid.spaceBetween)/e.value.style.chart.layout.grid.size),j=c(()=>k.value.width/e.value.style.chart.layout.grid.size),J=c(()=>k.value.height/e.value.style.chart.layout.grid.size);function ke(l){const o=e.value.style.chart.layout.grid.size*e.value.style.chart.layout.grid.size,a=l.reduce((w,ue)=>w+ue,0),t=l.map(w=>w/a*o),u=t.map(Math.floor),v=t.map(w=>w%1);let T=o-u.reduce((w,ue)=>w+ue,0);for(;T>0;){let w=v.indexOf(Math.max(...v));u[w]+=1,v[w]=0,T-=1}return u}function $e(){return b.dataset.map((l,o)=>({...l,color:ul(l.color)||xe.value[o]||S[o]||S[o%S.length],uid:`serie_${o}`,absoluteIndex:o}))}const Ce=c(()=>$e()),d=n(Ce.value);Le(()=>b.dataset,l=>{d.value=$e()},{deep:!0});const He=c(()=>{const l=d.value.filter((o,a)=>!y.value.includes(o.uid)).map((o,a)=>(o.values||[]).reduce((t,u)=>t+u,0));return ke(l)}),qe=c(()=>{const l=d.value.map((o,a)=>(o.values||[]).reduce((t,u)=>t+u));return ke(l)}),R=c(()=>(b.dataset.forEach((l,o)=>{[null,void 0].includes(l.values)&&se({componentName:"VueUiWaffle",type:"datasetSerieAttribute",property:"values (number[])",index:o})}),d.value.filter((l,o)=>!y.value.includes(l.uid)).map((l,o)=>({absoluteIndex:l.absoluteIndex,uid:l.uid,name:l.name,color:l.color,value:(l.values||[]).reduce((a,t)=>a+t,0),absoluteValues:l.values||[],proportion:He.value[o]})))),je=c(()=>d.value.map((l,o)=>({absoluteIndex:l.absoluteIndex,uid:l.uid,name:l.name,color:l.color,value:(l.values||[]).reduce((a,t)=>a+t,0),absoluteValues:l.values||[],proportion:qe.value[o]})));function Re(){return je.value.map(l=>({name:l.name,color:l.color,value:l.value,proportion:l.proportion}))}const De=c(()=>{let l=0;return R.value.map((o,a)=>{const t=l,u=t+o.proportion,v=[];for(let T=Math.floor(t);T<Math.floor(u);T+=1)v.push(T);return l=u,{...o,start:t,rects:v}})}),r=c(()=>De.value.flatMap((l,o)=>l.rects.map((a,t)=>({isFirst:t===0,isLongEnough:a.length>2,name:l.name,color:l.color,value:l.value,serieIndex:o,absoluteStartIndex:t<3,serieId:l.uid,...l}))).map((l,o)=>({...l,isAbsoluteFirst:o%e.value.style.chart.layout.grid.size===0}))),D=c(()=>{const l=[];for(let o=0;o<e.value.style.chart.layout.grid.size;o+=1)for(let a=0;a<e.value.style.chart.layout.grid.size;a+=1)l.push({isStartOfLine:a===0,position:e.value.style.chart.layout.grid.vertical?o:a,x:(e.value.style.chart.layout.grid.vertical?o:a)*(L.value+e.value.style.chart.layout.grid.spaceBetween),y:(e.value.style.chart.layout.grid.vertical?a:o)*(N.value+e.value.style.chart.layout.grid.spaceBetween)+k.value.top});return l}),y=n([]),P=n(!1),Ie=n(null),Te=n(null);function _e(l){if(!e.value.useAnimation){y.value.includes(l)?y.value=y.value.filter(t=>t!==l):y.value.length<M.value.length-1&&M.value.length>1&&y.value.push(l);return}const o=Ce.value.find(t=>t.uid===l).values.reduce((t,u)=>t+u,0);let a=d.value.find(t=>t.uid===l).values.reduce((t,u)=>t+u,0);if(y.value.includes(l)){let t=function(){a>u?(cancelAnimationFrame(Ie.value),d.value=d.value.map((v,T)=>v.uid===l?{...v,values:[u]}:v),P.value=!1):(P.value=!0,a+=u*.025,d.value=d.value.map((v,T)=>v.uid===l?{...v,values:[a]}:v),Ie.value=requestAnimationFrame(t))};y.value=y.value.filter(v=>v!==l);const u=o;t()}else if(y.value.length<M.value.length-1&&M.value.length>1){let t=function(){a<.1?(cancelAnimationFrame(Te.value),y.value.push(l),d.value=d.value.map((u,v)=>u.uid===l?{...u,values:[0]}:u),P.value=!1):(P.value=!0,a/=1.5,d.value=d.value.map((u,v)=>u.uid===l?{...u,values:[a]}:u),Te.value=requestAnimationFrame(t))};t()}Me("selectLegend",R.value.map(t=>({name:t.name,color:t.color,value:t.value,proportion:t.proportion/Math.pow(e.value.style.chart.layout.grid.size,2)})))}const M=c(()=>d.value.map((l,o)=>({name:l.name,color:l.color||xe[o]||S[o]||S[o%S.length],value:(l.values||[]).reduce((a,t)=>a+t,0),uid:l.uid,shape:"square"})).map((l,o)=>({...l,proportion:l.value/d.value.map(a=>(a.values||[]).reduce((t,u)=>t+u,0)).reduce((a,t)=>a+t,0),opacity:y.value.includes(l.uid)?.5:1,segregate:()=>_e(l.uid),isSegregated:y.value.includes(l.uid)}))),Ue=c(()=>({cy:"waffle-div-legend",backgroundColor:e.value.style.chart.legend.backgroundColor,color:e.value.style.chart.legend.color,fontSize:e.value.style.chart.legend.fontSize,paddingBottom:12,fontWeight:e.value.style.chart.legend.bold?"bold":""})),$=c(()=>R.value.map(l=>l.value).reduce((l,o)=>l+o,0)),ae=n(null);function Xe(l){if(y.value.length===b.dataset.length)return;const o=r.value[l];ae.value={datapoint:o,seriesIndex:o.absoluteIndex,series:d.value,config:e.value},ee.value=!0,O.value=r.value[l].serieIndex;const a=e.value.style.chart.tooltip.customFormat;if(fl(a)&&bl(()=>a({seriesIndex:r.value[l].absoluteIndex,datapoint:o,series:d.value,config:e.value})))le.value=a({seriesIndex:r.value[l].absoluteIndex,datapoint:o,series:d.value,config:e.value});else{let t="";if(t+=`<div style="width:100%;text-align:center;border-bottom:1px solid ${e.value.style.chart.tooltip.borderColor};padding-bottom:6px;margin-bottom:3px;">${o.name}</div>`,t+=`<div style="display:flex;flex-direction:row;gap:6px;align-items:center;"><svg viewBox="0 0 12 12" height="14" width="14"><rect x="0" y="0" height="12" width="12" stroke="none" rx="1" fill="${o.color}" /></svg>`,e.value.style.chart.tooltip.showValue&&(t+=`<b>${W(e.value.style.chart.layout.labels.dataLabels.formatter,o.value,x({p:e.value.style.chart.layout.labels.dataLabels.prefix,v:o.value,s:e.value.style.chart.layout.labels.dataLabels.suffix,r:e.value.style.chart.tooltip.roundingValue}),{datapoint:o,seriesIndex:r.value[l].absoluteIndex,series:d.value})}</b>`),e.value.style.chart.tooltip.showPercentage){const u=x({v:o.value/$.value*100,s:"%",r:e.value.style.chart.tooltip.roundingPercentage});e.value.style.chart.tooltip.showValue?t+=`<span>(${u})</span></div>`:t+=`<b>${u}%</b></div>`}le.value=t}}const C=c(()=>{const l=R.value.map(a=>({name:a.name,color:a.color})),o=R.value.map(a=>a.value);return{head:l,body:o}});function te(l){return e.value.useBlurOnHover&&![null,void 0].includes(O.value)&&O.value!==l?`url(#blur_${A.value})`:""}function ze(){sl(()=>{const l=C.value.head.map((t,u)=>[[t.name],[C.value.body[u]],[isNaN(C.value.body[u]/$.value)?"-":C.value.body[u]/$.value*100]]),o=[[e.value.style.chart.title.text],[e.value.style.chart.title.subtitle.text],[[""],["val"],["%"]]].concat(l),a=rl(o);il({csvContent:a,title:e.value.style.chart.title.text||"vue-ui-waffle"})})}const K=c(()=>{const l=[' <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor" fill="none" stroke-linecap="round" stroke-linejoin="round"><path stroke="none" d="M0 0h24v24H0z" fill="none"/><path d="M18 16v2a1 1 0 0 1 -1 1h-11l6 -7l-6 -7h11a1 1 0 0 1 1 1v2" /></svg>',W(e.value.style.chart.layout.labels.dataLabels.formatter,$.value,x({p:e.value.style.chart.layout.labels.dataLabels.prefix,v:$.value,s:e.value.style.chart.layout.labels.dataLabels.suffix,r:e.value.table.td.roundingValue})),"100%"],o=C.value.head.map((u,v)=>[{color:u.color,name:u.name},W(e.value.style.chart.layout.labels.dataLabels.formatter,C.value.body[v],x({p:e.value.style.chart.layout.labels.dataLabels.prefix,v:C.value.body[v],s:e.value.style.chart.layout.labels.dataLabels.suffix,r:e.value.table.td.roundingValue})),isNaN(C.value.body[v]/$.value)?"-":x({v:C.value.body[v]/$.value*100,s:"%",r:e.value.table.td.roundingPercentage})]),a={th:{backgroundColor:e.value.table.th.backgroundColor,color:e.value.table.th.color,outline:e.value.table.th.outline},td:{backgroundColor:e.value.table.td.backgroundColor,color:e.value.table.td.color,outline:e.value.table.td.outline},shape:"square",breakpoint:e.value.table.responsiveBreakpoint},t=[e.value.table.columnNames.series,e.value.table.columnNames.value,e.value.table.columnNames.percentage];return{head:l,body:o,config:a,colNames:t}}),U=n(!1);function Ye(l){U.value=l,re.value+=1}function Ae(){I.value.showTable=!I.value.showTable}function Be(){I.value.showTooltip=!I.value.showTooltip}const Q=n(!1);function oe(){Q.value=!Q.value}return Ve({getData:Re,generatePdf:me,generateCsv:ze,generateImage:we,toggleTable:Ae,toggleTooltip:Be,toggleAnnotator:oe}),(l,o)=>(s(),i("div",{class:Ne(`vue-ui-waffle ${U.value?"vue-data-ui-wrapper-fullscreen":""}`),ref_key:"waffleChart",ref:V,id:`vue-ui-waffle_${A.value}`,style:Z(`font-family:${e.value.style.fontFamily};width:100%; text-align:center;background:${e.value.style.chart.backgroundColor};${e.value.responsive?"height: 100%":""}`)},[e.value.userOptions.buttons.annotator?(s(),F(Bl,{key:0,parent:V.value,backgroundColor:e.value.style.chart.backgroundColor,color:e.value.style.chart.color,active:Q.value,onClose:oe},null,8,["parent","backgroundColor","color","active"])):p("",!0),Ge.value?(s(),i("div",{key:1,ref_key:"noTitle",ref:ce,class:"vue-data-ui-no-title-space",style:"height:36px; width: 100%;background:transparent"},null,512)):p("",!0),e.value.style.chart.title.text?(s(),i("div",{key:2,ref_key:"chartTitle",ref:ie,style:"width:100%;background:transparent;padding-bottom:12px"},[(s(),F(xl,{key:`title_${de.value}`,config:{title:{cy:"waffle-title",...e.value.style.chart.title},subtitle:{cy:"waffle-subtitle",...e.value.style.chart.title.subtitle}}},null,8,["config"]))],512)):p("",!0),e.value.userOptions.show&&X.value?(s(),F($l,{ref_key:"details",ref:Ee,key:`user_options_${re.value}`,backgroundColor:e.value.style.chart.backgroundColor,color:e.value.style.chart.color,isPrinting:h(fe),isImaging:h(be),uid:A.value,hasTooltip:e.value.userOptions.buttons.tooltip&&e.value.style.chart.tooltip.show,hasPdf:e.value.userOptions.buttons.pdf,hasImg:e.value.userOptions.buttons.img,hasXls:e.value.userOptions.buttons.csv,hasTable:e.value.userOptions.buttons.table,hasFullscreen:e.value.userOptions.buttons.fullscreen,isFullscreen:U.value,isTooltip:I.value.showTooltip,titles:{...e.value.userOptions.buttonTitles},chartElement:V.value,position:e.value.userOptions.position,hasAnnotator:e.value.userOptions.buttons.annotator,isAnnotation:Q.value,onToggleFullscreen:Ye,onGeneratePdf:h(me),onGenerateCsv:ze,onGenerateImage:h(we),onToggleTable:Ae,onToggleTooltip:Be,onToggleAnnotator:oe},nl({_:2},[l.$slots.optionTooltip?{name:"optionTooltip",fn:f(()=>[g(l.$slots,"optionTooltip",{},void 0,!0)]),key:"0"}:void 0,l.$slots.optionPdf?{name:"optionPdf",fn:f(()=>[g(l.$slots,"optionPdf",{},void 0,!0)]),key:"1"}:void 0,l.$slots.optionCsv?{name:"optionCsv",fn:f(()=>[g(l.$slots,"optionCsv",{},void 0,!0)]),key:"2"}:void 0,l.$slots.optionImg?{name:"optionImg",fn:f(()=>[g(l.$slots,"optionImg",{},void 0,!0)]),key:"3"}:void 0,l.$slots.optionTable?{name:"optionTable",fn:f(()=>[g(l.$slots,"optionTable",{},void 0,!0)]),key:"4"}:void 0,l.$slots.optionFullscreen?{name:"optionFullscreen",fn:f(({toggleFullscreen:a,isFullscreen:t})=>[g(l.$slots,"optionFullscreen",E(G({toggleFullscreen:a,isFullscreen:t})),void 0,!0)]),key:"5"}:void 0,l.$slots.optionAnnotator?{name:"optionAnnotator",fn:f(({toggleAnnotator:a,isAnnotator:t})=>[g(l.$slots,"optionAnnotator",E(G({toggleAnnotator:a,isAnnotator:t})),void 0,!0)]),key:"6"}:void 0]),1032,["backgroundColor","color","isPrinting","isImaging","uid","hasTooltip","hasPdf","hasImg","hasXls","hasTable","hasFullscreen","isFullscreen","isTooltip","titles","chartElement","position","hasAnnotator","isAnnotation","onGeneratePdf","onGenerateImage"])):p("",!0),X.value?(s(),i("svg",{key:4,xmlns:h(vl),class:Ne({"vue-data-ui-fullscreen--on":U.value,"vue-data-ui-fulscreen--off":!U.value}),viewBox:`0 0 ${B.value.width<=0?10:B.value.width} ${B.value.height<=0?10:B.value.height}`,style:Z(`max-width:100%;overflow:visible;background:transparent;color:${e.value.style.chart.color}`)},[Pe(Al),m("defs",null,[(s(!0),i(_,null,H(r.value,(a,t)=>(s(),i("radialGradient",{cx:"50%",cy:"50%",r:"50%",fx:"50%",fy:"50%",id:`gradient_${A.value}_${t}`},[m("stop",{offset:"0%","stop-color":h(cl)(h(dl)(a.color,.05),100-e.value.style.chart.layout.rect.gradientIntensity)},null,8,Fl),m("stop",{offset:"100%","stop-color":a.color},null,8,Ol)],8,Sl))),256))]),m("defs",null,[m("filter",{id:`blur_${A.value}`,x:"-50%",y:"-50%",width:"200%",height:"200%"},El,8,Vl)]),e.value.useCustomCells&&r.value.length?(s(!0),i(_,{key:0},H(D.value,(a,t)=>(s(),i("foreignObject",{x:a.x,y:a.y,height:N.value<=0?1e-4:N.value,width:L.value<=0?1e-4:L.value,class:"vue-ui-waffle-custom-cell-foreignObject"},[g(l.$slots,"cell",E(G({cell:{...a,color:r.value[t].color,...r.value[t]},isSelected:[null,void 0].includes(O.value)?!0:r.value[t].serieIndex===O.value})),void 0,!0)],8,Gl))),256)):p("",!0),!r.value.length&&!e.value.useCustomCells?(s(),i("rect",{key:1,x:12,y:12,height:k.value.height-24,width:k.value.width-24,rx:3,fill:"none",stroke:"black"},null,8,Hl)):r.value.length&&!e.value.useCustomCells?(s(),i(_,{key:2},[(s(!0),i(_,null,H(D.value,(a,t)=>(s(),i("rect",{rx:e.value.style.chart.layout.rect.rounded?e.value.style.chart.layout.rect.rounding:0,x:a.x+e.value.style.chart.layout.grid.spaceBetween/2,y:a.y+e.value.style.chart.layout.grid.spaceBetween/2,height:N.value<=0?1e-4:N.value,width:L.value<=0?1e-4:L.value,fill:"white",stroke:e.value.style.chart.layout.rect.stroke,"stroke-width":e.value.style.chart.layout.rect.strokeWidth,filter:te(r.value[t].serieIndex)},null,8,ql))),256)),(s(!0),i(_,null,H(D.value,(a,t)=>(s(),i("rect",{rx:e.value.style.chart.layout.rect.rounded?e.value.style.chart.layout.rect.rounding:0,x:a.x+e.value.style.chart.layout.grid.spaceBetween/2,y:a.y+e.value.style.chart.layout.grid.spaceBetween/2,height:N.value<=0?1e-4:N.value,width:L.value<=0?1e-4:L.value,fill:e.value.style.chart.layout.rect.useGradient&&e.value.style.chart.layout.rect.gradientIntensity>0?`url(#gradient_${A.value}_${t})`:r.value[t].color,stroke:e.value.style.chart.layout.rect.stroke,"stroke-width":e.value.style.chart.layout.rect.strokeWidth,filter:te(r.value[t].serieIndex)},null,8,jl))),256))],64)):p("",!0),(s(!0),i(_,null,H(D.value,(a,t)=>(s(),i(_,null,[r.value.length&&!P.value&&!e.value.style.chart.layout.grid.vertical&&e.value.style.chart.layout.labels.captions.show&&(r.value[t].isFirst&&a.position<e.value.style.chart.layout.grid.size-2||r.value[t].isAbsoluteFirst&&t%e.value.style.chart.layout.grid.size===0&&r.value[t].absoluteStartIndex)?(s(),i("foreignObject",{key:0,x:a.x+e.value.style.chart.layout.labels.captions.offsetX+e.value.style.chart.layout.grid.spaceBetween/2,y:a.y+e.value.style.chart.layout.labels.captions.offsetY+e.value.style.chart.layout.grid.spaceBetween/2,height:J.value<=0?1e-4:J.value,width:j.value*e.value.style.chart.layout.grid.size<=0?1e-4:j.value*e.value.style.chart.layout.grid.size,filter:te(r.value[t].serieIndex)},[m("div",{class:"vue-ui-waffle-caption",style:Z(`height: 100%; width: 100%; font-size:${e.value.style.chart.layout.labels.captions.fontSize}px;display:flex;align-items:center;justify-content:flex-start;padding: 0 ${j.value/12}px;color:${h(gl)(r.value[t].color)};gap:2px`)},[e.value.style.chart.layout.labels.captions.showSerieName?(s(),i("span",Dl,z(e.value.style.chart.layout.labels.captions.serieNameAbbreviation?h(pl)({source:r.value[t].name,length:e.value.style.chart.layout.labels.captions.serieNameMaxAbbreviationSize}):r.value[t].name)+": ",1)):p("",!0),e.value.style.chart.layout.labels.captions.showPercentage?(s(),i("span",Ul,z(h(x)({v:r.value[t].proportion,s:"%",r:e.value.style.chart.layout.labels.captions.roundingPercentage})),1)):p("",!0),e.value.style.chart.layout.labels.captions.showPercentage&&e.value.style.chart.layout.labels.captions.showValue?(s(),i("span",Xl," ("+z(h(W)(e.value.style.chart.layout.labels.dataLabels.formatter,r.value[t].value,h(x)({p:e.value.style.chart.layout.labels.dataLabels.prefix,v:r.value[t].value,s:e.value.style.chart.layout.labels.dataLabels.suffix,r:e.value.style.chart.layout.labels.captions.roundingValue}),{datapoint:r.value[t],position:a}))+") ",1)):p("",!0),!e.value.style.chart.layout.labels.captions.showPercentage&&e.value.style.chart.layout.labels.captions.showValue?(s(),i("span",Yl,z(h(W)(e.value.style.chart.layout.labels.dataLabels.formatter,r.value[t].value,h(x)({p:e.value.style.chart.layout.labels.dataLabels.prefix,v:r.value[t].value,s:e.value.style.chart.layout.labels.dataLabels.suffix,r:e.value.style.chart.layout.labels.captions.roundingValue}),{datapoint:r.value[t],position:a})),1)):p("",!0)],4)],8,Rl)):p("",!0)],64))),256)),(s(!0),i(_,null,H(D.value,(a,t)=>(s(),i("rect",{onMouseover:u=>Xe(t),onMouseleave:o[0]||(o[0]=u=>{ee.value=!1,O.value=null}),x:a.x+e.value.style.chart.layout.grid.spaceBetween/2,y:a.y+e.value.style.chart.layout.grid.spaceBetween/2,height:J.value<=0?1e-4:J.value,width:j.value<=0?1e-4:j.value,fill:"transparent",stroke:"none"},null,40,Jl))),256)),g(l.$slots,"svg",{svg:B.value},void 0,!0)],14,Pl)):p("",!0),l.$slots.watermark?(s(),i("div",Kl,[g(l.$slots,"watermark",E(G({isPrinting:h(fe)||h(be)})),void 0,!0)])):p("",!0),X.value?p("",!0):(s(),F(_l,{key:6,config:{type:"waffle",style:{backgroundColor:e.value.style.chart.backgroundColor,waffle:{color:"#CCCCCC"}}}},null,8,["config"])),m("div",{ref_key:"chartLegend",ref:ne},[e.value.style.chart.legend.show?(s(),F(Tl,{key:`legend_${ye.value}`,legendSet:M.value,config:Ue.value,onClickMarker:o[1]||(o[1]=({legend:a})=>_e(a.uid))},{item:f(({legend:a})=>[m("div",{onClick:t=>a.segregate(),style:Z(`opacity:${y.value.includes(a.uid)?.5:1}`)},[Se(z(a.name)+": "+z(h(W)(e.value.style.chart.layout.labels.dataLabels.formatter,a.value,h(x)({p:e.value.style.chart.layout.labels.dataLabels.prefix,v:a.value,s:e.value.style.chart.layout.labels.dataLabels.suffix,r:e.value.style.chart.legend.roundingValue,isAnimating:P.value}),{datapoint:a}))+" ",1),y.value.includes(a.uid)?(s(),i("span",ea," ( - % ) ")):(s(),i("span",Zl," ("+z(isNaN(a.value/$.value)?"-":h(x)({v:a.value/$.value*100,s:"%",r:e.value.style.chart.legend.roundingPercentage,isAnimating:P.value}))+") ",1))],12,Ql)]),_:1},8,["legendSet","config"])):g(l.$slots,"legend",{key:1,legend:M.value},void 0,!0)],512),l.$slots.source?(s(),i("div",{key:7,ref_key:"source",ref:ve,dir:"auto"},[g(l.$slots,"source",{},void 0,!0)],512)):p("",!0),Pe(Cl,{show:I.value.showTooltip&&ee.value&&y.value.length<b.dataset.length,backgroundColor:e.value.style.chart.tooltip.backgroundColor,color:e.value.style.chart.tooltip.color,borderRadius:e.value.style.chart.tooltip.borderRadius,borderColor:e.value.style.chart.tooltip.borderColor,borderWidth:e.value.style.chart.tooltip.borderWidth,backgroundOpacity:e.value.style.chart.tooltip.backgroundOpacity,position:e.value.style.chart.tooltip.position,offsetY:e.value.style.chart.tooltip.offsetY,parent:V.value,content:le.value,isCustom:e.value.style.chart.tooltip.customFormat&&typeof e.value.style.chart.tooltip.customFormat=="function",fontSize:e.value.style.chart.tooltip.fontSize},{"tooltip-before":f(()=>[g(l.$slots,"tooltip-before",E(G({...ae.value})),void 0,!0)]),"tooltip-after":f(()=>[g(l.$slots,"tooltip-after",E(G({...ae.value})),void 0,!0)]),_:3},8,["show","backgroundColor","color","borderRadius","borderColor","borderWidth","backgroundOpacity","position","offsetY","parent","content","isCustom","fontSize"]),X.value?(s(),F(zl,{key:8,hideDetails:"",config:{open:I.value.showTable,maxHeight:1e4,body:{backgroundColor:e.value.style.chart.backgroundColor,color:e.value.style.chart.color},head:{backgroundColor:e.value.style.chart.backgroundColor,color:e.value.style.chart.color}}},{content:f(()=>[(s(),F(Il,{key:`table_${he.value}`,colNames:K.value.colNames,head:K.value.head,body:K.value.body,config:K.value.config,title:`${e.value.style.chart.title.text}${e.value.style.chart.title.subtitle.text?` : ${e.value.style.chart.title.subtitle.text}`:""}`,onClose:o[2]||(o[2]=a=>I.value.showTable=!1)},{th:f(({th:a})=>[m("div",{innerHTML:a,style:{display:"flex","align-items":"center"}},null,8,la)]),td:f(({td:a})=>[Se(z(a.name||a),1)]),_:1},8,["colNames","head","body","config","title"]))]),_:1},8,["config"])):p("",!0)],14,Nl))}},fa=Ll(aa,[["__scopeId","data-v-b51c7605"]]);export{fa as default};
