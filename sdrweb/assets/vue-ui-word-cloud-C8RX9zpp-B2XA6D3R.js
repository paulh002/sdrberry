import{z as Ee,c as p,r as s,aJ as Be,a0 as We,D as fe,R as je,A as L,H as De,w as te,B as Ge,C as ye,Z as He,K as be,b as Le,ac as Ue,G as qe,V as Je,N as Ye,o as u,a as h,i as A,L as w,j as Xe,l as d,m as c,p as N,q as P,u as x,J as Ze,n as we,f as oe,e as $,P as Ke,k as Qe,M as ae,a2 as et,h as ke,af as tt,S as ot,T as at,g as lt}from"./index-DIL_pLmJ.js";import{x as st}from"./Title-DyD-mBPN-CHqvPevn.js";import{r as rt,a as nt}from"./usePrinter-CLcxWv-4-CCDxi2Mm.js";import{O as ut,k as it,E as vt}from"./useResponsive-CoxXLe23-CoJWR8uA.js";import ct from"./vue-ui-accordion-m8rut1bP-DBR24duA.js";import{W as dt}from"./DataTable-D8fcY_sJ-By-GDpuN.js";import{s as xe,u as ht}from"./vue-data-ui-DF7u-eZV-hcmE5A1B.js";import{c as Ce}from"./useNestedProp-BxrcwmTZ-1Bk3HyKT.js";import{p as mt}from"./PackageVersion-mSsg8dpu-BxnQ4_xj.js";import{E as pt}from"./Tooltip-qod9o4Vz-Dsvb9C65.js";import{S as gt}from"./PenAndPaper-Dk41ZrJ0-D5uht0-m.js";import"./Shape-DYFnvMNd-DLTmTYE_.js";const ft=v=>(ot("data-v-3fd7d274"),v=v(),at(),v),yt={"data-html2canvas-ignore":""},bt={key:0,class:"reset-wrapper"},wt={key:1,class:"reset-wrapper"},kt={class:"mono-slicer"},Ct=ft(()=>$("div",{class:"slider-track"},null,-1)),xt=["min","max","value"],$t={__name:"MonoSlicer",props:{min:{type:Number,default:0},max:{type:Number,default:0},inputColor:{type:String,default:"#1A1A1A"},background:{type:String,default:"#FFFFFF"},borderColor:{type:String,default:"#FFFFFF"},selectColor:{type:String,default:"#4A4A4A"},textColor:{type:String,default:"#1A1A1A"},useResetSlot:{type:Boolean,default:!1},value:{type:Number,default:0},source:{type:Number,default:0}},emits:["update:value","reset"],setup(v,{emit:V}){const r=v;tt(C=>({"90ad3208":U.value,"4f9eef70":O.value,"6e07ae3e":k.value,"2fd430a0":v.selectColor,"6b8169a4":S.value}));const U=p(()=>r.inputColor),S=p(()=>r.background),k=p(()=>`${r.selectColor}33`),O=p(()=>r.borderColor);function I(){V("reset")}const f=p(()=>{const C=r.max-r.min,y=0,T=(r.value-r.min)/C*100;return{left:`${y}%`,width:`${T-y}%`,background:r.selectColor}});return(C,y)=>(u(),h("div",yt,[v.value!==v.source?(u(),h("div",bt,[v.useResetSlot?c(C.$slots,"reset-action",{key:1,reset:I},void 0,!0):(u(),h("button",{key:0,"data-cy-reset":"",tabindex:"0",role:"button",class:"vue-data-ui-refresh-button",onClick:I},[oe(ht,{name:"refresh",stroke:v.textColor},null,8,["stroke"])]))])):(u(),h("div",wt)),$("div",kt,[Ct,$("div",{class:"range-highlight",style:ae(f.value)},null,4),$("input",{type:"range",min:v.min,max:v.max,value:Number(v.value),onInput:y[0]||(y[0]=T=>V("update:value",Number(T.target.value)))},null,40,xt)])]))}},St=xe($t,[["__scopeId","data-v-3fd7d274"]]),Tt=["id"],Ft=["xmlns","viewBox"],_t=["transform"],zt=["fill","font-weight","x","y","font-size","transform","onMouseover"],At={key:4,class:"vue-data-ui-watermark"},Nt=["innerHTML"],Pt={__name:"vue-ui-word-cloud",props:{config:{type:Object,default(){return{}}},dataset:{type:[Array,String],default(){return[]}}},setup(v,{expose:V}){const r=v,{vue_ui_word_cloud:U}=Ee(),S=p({get(){return!!r.dataset&&r.dataset.length},set(t){return t}}),k=s(typeof r.dataset=="string"?Be(r.dataset):r.dataset.map(t=>({...t,value:We(t.value)}))),O=s(fe()),I=s(0),f=s(null),C=s(null),y=s(null),T=s(0),le=s(0),q=s(!1),e=p({get:()=>se(),set:t=>t});function se(){const t=Ce({userConfig:r.config,defaultConfig:U});return t.theme?{...Ce({userConfig:De.vue_ui_word_cloud[t.theme]||r.config,defaultConfig:t}),customPalette:je[t.theme]||L}:t}te(()=>r.config,t=>{e.value=se(),ne(),T.value+=1,le.value+=1,J()},{deep:!0});const re=s(null),F=s(0),i=s({width:e.value.style.chart.width,height:e.value.style.chart.height,maxFontSize:e.value.style.chart.words.maxFontSize,minFontSize:e.value.style.chart.words.minFontSize}),$e=ut(()=>{const{width:t,height:a}=it({chart:f.value,title:e.value.style.chart.title.text?C.value:null,slicer:e.value.style.chart.zoom.show&&re.value,source:y.value});i.value.width=t,i.value.height=a,be(Y)});te(()=>F.value,()=>{Se()});const Se=vt(()=>{Y()},10);function J(){F.value=de.value}const E=s(null);Le(()=>{ne(),J()});function ne(){Ge(r.dataset)?ye({componentName:"VueUiWordCloud",type:"dataset"}):k.value.forEach((t,a)=>{He({datasetObject:t,requiredAttributes:["name","value"]}).forEach(o=>{S.value=!1,ye({componentName:"VueUiWordCloud",type:"datasetSerieAttribute",property:o,index:a})})}),e.value.responsive&&(E.value=new ResizeObserver($e),E.value.observe(f.value.parentNode))}Ue(()=>{E.value&&E.value.disconnect()});const{isPrinting:ue,isImaging:ie,generatePdf:ve,generateImage:ce}=rt({elementId:`wordCloud_${O.value}`,fileName:e.value.style.chart.title.text||"vue-ui-word-cloud"}),m=s({showTable:e.value.table.show,showTooltip:e.value.style.chart.tooltip.show});function Te(t,a,o="Arial"){const n=document.createElement("canvas").getContext("2d");return n.font=`${a}px ${e.value.style.chart.words.bold?"bold":"normal"} ${o}`,{width:n.measureText(t).width+e.value.style.chart.words.proximity,height:a}}function Fe(t,a){return t.x<a.x+a.width&&t.x+t.width>a.x&&t.y<a.y+a.height&&t.y+t.height>a.y}function _e(t,a,o){const n=[],l={x:-a/2,y:-o/2,width:a,height:o},g=0,G=0;return t.forEach(z=>{let Q=!1;for(let H=0;H<Math.max(a,o)/2&&!Q;H+=e.value.style.chart.words.packingWeight)for(let ee=0;ee<360&&!Q;ee+=e.value.style.chart.words.packingWeight){const ge=ee*Math.PI/180,Oe=g+H*Math.cos(ge)-z.width/2,Ie=G+H*Math.sin(ge)-z.height/2,b={...z,x:Oe,y:Ie},Me=b.x>=l.x&&b.y>=l.y&&b.x+b.width<=l.x+l.width&&b.y+b.height<=l.y+l.height,Re=n.some(Ve=>Fe(b,Ve));Me&&!Re&&(n.push(b),Q=!0)}}),n}const M=s([]);te(()=>r.dataset,Y,{immediate:!0});const de=p(()=>Math.min(...k.value.map(t=>t.value))),ze=p(()=>Math.max(...k.value.map(t=>t.value)));function Y(){const t=[...k.value].filter(l=>l.value>=F.value).map(l=>l.value),a=Math.max(...t),o=Math.min(...t),n=[...k.value].filter(l=>l.value>=F.value).map((l,g)=>{const G=(l.value-o)/(a-o)*(i.value.maxFontSize-i.value.minFontSize)+i.value.minFontSize,z=Te(l.name,G);return{...l,id:fe(),fontSize:G,width:z.width,height:z.height,color:e.value.style.chart.words.usePalette?e.value.customPalette[g]||e.value.customPalette[g%e.value.customPalette.length]||L[g]||L[g%L.length]:e.value.style.chart.words.color}});M.value=_e(n,i.value.width,i.value.height).sort((l,g)=>g.fontSize-l.fontSize)}const B=p(()=>{const t=M.value.map(o=>({name:o.name,color:o.color})),a=M.value.map(o=>o.value);return{head:t,body:a}});function he(){be(()=>{const t=B.value.head.map((n,l)=>[[n.name],[B.value.body[l]]]),a=[[e.value.style.chart.title.text],[e.value.style.chart.title.subtitle.text],[[""],[e.value.table.columnNames.value]]].concat(t),o=Je(a);Ye({csvContent:o,title:e.value.style.chart.title.text||"vue-ui-word-cloud"})})}const W=p(()=>{const t=[e.value.table.columnNames.series,e.value.table.columnNames.value],a=B.value.head.map((n,l)=>{const g=qe({p:e.value.table.td.prefix,v:B.value.body[l],s:e.value.table.td.suffix,r:e.value.table.td.roundingValue});return[{color:n.color,name:n.name},g]}),o={th:{backgroundColor:e.value.table.th.backgroundColor,color:e.value.table.th.color,outline:e.value.table.th.outline},td:{backgroundColor:e.value.table.td.backgroundColor,color:e.value.table.td.color,outline:e.value.table.td.outline},breakpoint:e.value.table.responsiveBreakpoint};return{colNames:[e.value.table.columnNames.series,e.value.table.columnNames.value],head:t,body:a,config:o}}),j=s(!1);function Ae(t){j.value=t,I.value+=1}function Ne(){return M.value}function me(){m.value.showTable=!m.value.showTable}function pe(){m.value.showTooltip=!m.value.showTooltip}const D=s(!1);function X(){D.value=!D.value}V({getData:Ne,generateCsv:he,generatePdf:ve,generateImage:ce,toggleTable:me,toggleTooltip:pe,toggleAnnotator:X});const _=s(null),R=s(!1),Z=s(""),K=s(null);function Pe(t){if(!m.value.showTooltip)return;_.value=t.id,K.value={datapoint:t,config:e.value};const a=e.value.style.chart.tooltip.customFormat;if(R.value=!1,et(a))try{const o=a({datapoint:t,config:e.value});typeof o=="string"&&(Z.value=o,R.value=!0)}catch{console.warn("Custom format cannot be applied."),R.value=!1}if(!R.value){let o=`<svg viewBox="0 0 10 10" height="${e.value.style.chart.tooltip.fontSize}"><circle cx="5" cy="5" r="5" fill="${t.color}"/></svg><span>${t.name}:</span><b>${(t.value||0).toFixed(e.value.style.chart.tooltip.roundingValue)}</b>`;Z.value=`<div dir="auto" style="display:flex; gap:4px; align-items:center; jsutify-content:center;">${o}</div>`}q.value=!0}return(t,a)=>(u(),h("div",{class:"vue-ui-word-cloud",ref_key:"wordCloudChart",ref:f,id:`wordCloud_${O.value}`,style:ae(`width: 100%; font-family:${e.value.style.fontFamily};background:${e.value.style.chart.backgroundColor};${e.value.responsive?"height:100%":""}`)},[e.value.userOptions.buttons.annotator?(u(),A(gt,{key:0,parent:f.value,backgroundColor:e.value.style.chart.backgroundColor,color:e.value.style.chart.color,active:D.value,onClose:X},null,8,["parent","backgroundColor","color","active"])):w("",!0),e.value.style.chart.title.text?(u(),h("div",{key:1,ref_key:"chartTitle",ref:C,style:"width:100%;background:transparent;padding-bottom:24px"},[(u(),A(st,{key:`title_${T.value}`,config:{title:{...e.value.style.chart.title},subtitle:{...e.value.style.chart.title.subtitle}}},null,8,["config"]))],512)):w("",!0),e.value.userOptions.show&&S.value?(u(),A(nt,{ref:"details",key:`user_option_${I.value}`,backgroundColor:e.value.style.chart.backgroundColor,color:e.value.style.chart.color,isPrinting:x(ue),isImaging:x(ie),uid:O.value,hasPdf:e.value.userOptions.buttons.pdf,hasXls:e.value.userOptions.buttons.csv,hasImg:e.value.userOptions.buttons.img,hasTable:e.value.userOptions.buttons.table,hasFullscreen:e.value.userOptions.buttons.fullscreen,isFullscreen:j.value,titles:{...e.value.userOptions.buttonTitles},chartElement:f.value,position:e.value.userOptions.position,hasTooltip:e.value.style.chart.tooltip.show&&e.value.userOptions.buttons.tooltip,isTooltip:m.value.showTooltip,hasAnnotator:e.value.userOptions.buttons.annotator,isAnnotation:D.value,onToggleFullscreen:Ae,onGeneratePdf:x(ve),onGenerateCsv:he,onGenerateImage:x(ce),onToggleTable:me,onToggleTooltip:pe,onToggleAnnotator:X},Xe({_:2},[t.$slots.optionPdf?{name:"optionPdf",fn:d(()=>[c(t.$slots,"optionPdf",{},void 0,!0)]),key:"0"}:void 0,t.$slots.optionCsv?{name:"optionCsv",fn:d(()=>[c(t.$slots,"optionCsv",{},void 0,!0)]),key:"1"}:void 0,t.$slots.optionImg?{name:"optionImg",fn:d(()=>[c(t.$slots,"optionImg",{},void 0,!0)]),key:"2"}:void 0,t.$slots.optionTable?{name:"optionTable",fn:d(()=>[c(t.$slots,"optionTable",{},void 0,!0)]),key:"3"}:void 0,t.$slots.optionFullscreen?{name:"optionFullscreen",fn:d(({toggleFullscreen:o,isFullscreen:n})=>[c(t.$slots,"optionFullscreen",N(P({toggleFullscreen:o,isFullscreen:n})),void 0,!0)]),key:"4"}:void 0,t.$slots.optionAnnotator?{name:"optionAnnotator",fn:d(({toggleAnnotator:o,isAnnotator:n})=>[c(t.$slots,"optionAnnotator",N(P({toggleAnnotator:o,isAnnotator:n})),void 0,!0)]),key:"5"}:void 0]),1032,["backgroundColor","color","isPrinting","isImaging","uid","hasPdf","hasXls","hasImg","hasTable","hasFullscreen","isFullscreen","titles","chartElement","position","hasTooltip","isTooltip","hasAnnotator","isAnnotation","onGeneratePdf","onGenerateImage"])):w("",!0),S.value?(u(),h("svg",{key:3,class:we({"vue-data-ui-fullscreen--on":j.value,"vue-data-ui-fulscreen--off":!j.value}),xmlns:x(Ze),viewBox:`0 0 ${i.value.width<=0?10:i.value.width} ${i.value.height<=0?10:i.value.height}`,style:"overflow:visible;background:transparent;"},[oe(mt),$("g",{transform:`translate(${(i.value.width<=0?10:i.value.width)/2}, ${(i.value.height<=0?10:i.value.height)/2})`},[(u(!0),h(Ke,null,Qe(M.value,(o,n)=>(u(),h("g",null,[(u(),h("text",{fill:o.color,"font-weight":e.value.style.chart.words.bold?"bold":"normal",key:n,x:o.x,y:o.y,"font-size":o.fontSize,transform:`translate(${o.width/2}, ${o.height/2})`,class:we({animated:e.value.useCssAnimation,"word-selected":_.value&&_.value===o.id&&m.value.showTooltip,"word-not-selected":_.value&&_.value!==o.id&&m.value.showTooltip}),"text-anchor":"middle","dominant-baseline":"central",onMouseover:l=>Pe(o),onMouseleave:a[0]||(a[0]=l=>{_.value=null,q.value=!1}),style:ae(`animation-delay:${n*e.value.animationDelayMs}ms !important;`)},ke(o.name),47,zt))]))),256))],8,_t),c(t.$slots,"svg",{svg:{height:i.value.height,width:i.value.width}},void 0,!0)],10,Ft)):w("",!0),t.$slots.watermark?(u(),h("div",At,[c(t.$slots,"watermark",N(P({isPrinting:x(ue)||x(ie)})),void 0,!0)])):w("",!0),oe(pt,{show:m.value.showTooltip&&q.value,backgroundColor:e.value.style.chart.tooltip.backgroundColor,color:e.value.style.chart.tooltip.color,fontSize:e.value.style.chart.tooltip.fontSize,borderRadius:e.value.style.chart.tooltip.borderRadius,borderColor:e.value.style.chart.tooltip.borderColor,borderWidth:e.value.style.chart.tooltip.borderWidth,backgroundOpacity:e.value.style.chart.tooltip.backgroundOpacity,position:e.value.style.chart.tooltip.position,offsetY:e.value.style.chart.tooltip.offsetY,parent:f.value,content:Z.value,isCustom:R.value},{"tooltip-before":d(()=>[c(t.$slots,"tooltip-before",N(P({...K.value})),void 0,!0)]),"tooltip-after":d(()=>[c(t.$slots,"tooltip-after",N(P({...K.value})),void 0,!0)]),_:3},8,["show","backgroundColor","color","fontSize","borderRadius","borderColor","borderWidth","backgroundOpacity","position","offsetY","parent","content","isCustom"]),$("div",{ref_key:"chartSlicer",ref:re,style:"width:100%;background:transparent","data-html2canvas-ignore":""},[e.value.style.chart.zoom.show?(u(),A(St,{key:0,value:F.value,"onUpdate:value":a[1]||(a[1]=o=>F.value=o),min:de.value,max:ze.value,textColor:e.value.style.chart.color,inputColor:e.value.style.chart.zoom.color,selectColor:e.value.style.chart.zoom.highlightColor,useResetSlot:e.value.style.chart.zoom.useResetSlot,background:e.value.style.chart.zoom.color,borderColor:e.value.style.chart.backgroundColor,source:e.value.style.chart.width,onReset:J},{"reset-action":d(({reset:o})=>[c(t.$slots,"reset-action",N(P({reset:o})),void 0,!0)]),_:3},8,["value","min","max","textColor","inputColor","selectColor","useResetSlot","background","borderColor","source"])):w("",!0)],512),t.$slots.source?(u(),h("div",{key:5,ref_key:"source",ref:y,dir:"auto"},[c(t.$slots,"source",{},void 0,!0)],512)):w("",!0),S.value?(u(),A(ct,{key:6,hideDetails:"",config:{open:m.value.showTable,maxHeight:1e4,body:{backgroundColor:e.value.style.chart.backgroundColor,color:e.value.style.chart.color},head:{backgroundColor:e.value.style.chart.backgroundColor,color:e.value.style.chart.color}}},{content:d(()=>[(u(),A(dt,{key:`table_${le.value}`,colNames:W.value.colNames,head:W.value.head,body:W.value.body,config:W.value.config,title:`${e.value.style.chart.title.text}${e.value.style.chart.title.subtitle.text?` : ${e.value.style.chart.title.subtitle.text}`:""}`,onClose:a[2]||(a[2]=o=>m.value.showTable=!1)},{th:d(({th:o})=>[$("div",{innerHTML:o,style:{display:"flex","align-items":"center"}},null,8,Nt)]),td:d(({td:o})=>[lt(ke(o.name||o),1)]),_:1},8,["colNames","head","body","config","title"]))]),_:1},8,["config"])):w("",!0)],12,Tt))}},Lt=xe(Pt,[["__scopeId","data-v-13d267e2"]]);export{Lt as default};
