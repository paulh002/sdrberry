import{af as Ie,z as Ve,c as h,B as Fe,C as we,r as p,D as oe,H as Pe,ao as Be,w as ke,b as Re,Z as je,ap as se,K as Ee,V as Ge,N as He,o,a as u,i as E,L as c,j as Ye,l as m,m as f,p as G,q as H,u as n,M as w,n as J,J as We,f as ue,P as _,k as S,e as i,ag as ie,h as b,U as I,G as O,a2 as Xe,a3 as Ue,a0 as z,ah as qe,g as Ke}from"./index-DIL_pLmJ.js";import{x as Je}from"./Title-DyD-mBPN-CHqvPevn.js";import{r as Ze,a as De}from"./usePrinter-CLcxWv-4-CCDxi2Mm.js";import{E as Qe}from"./Tooltip-qod9o4Vz-Dsvb9C65.js";import el from"./vue-ui-skeleton-Dgzcn3FL-BNvPX0g2.js";import{s as ll,u as tl}from"./vue-data-ui-DF7u-eZV-hcmE5A1B.js";import al from"./vue-ui-accordion-m8rut1bP-DBR24duA.js";import{c as $e}from"./useNestedProp-BxrcwmTZ-1Bk3HyKT.js";import{p as ol}from"./PackageVersion-mSsg8dpu-BxnQ4_xj.js";import{S as sl}from"./PenAndPaper-Dk41ZrJ0-D5uht0-m.js";const ul=["id"],il={key:2,style:"width:100%;background:transparent"},nl=["xmlns","viewBox"],vl=["x","y","width","height","fill","stroke","stroke-width"],rl=["x","y","width","height","fill","stroke","stroke-width"],dl=["font-size","font-weight","fill","x","y"],yl=["x","y","width","height","onMouseover"],cl={key:0},hl=["font-size","fill","x","y","font-weight"],pl={key:0},gl=["text-anchor","font-size","fill","font-weight","transform"],fl={key:1},bl=["x","y","width","height","stroke","stroke-width"],xl={key:2},ml=["id"],wl=["stop-color"],kl=["stop-color"],$l=["x","y","font-size","fill"],Ll=["x","y","height","rx","fill"],Al=["x","y","font-size","fill"],Cl=["stroke","x1","x2","y1","y2"],_l=["fill","d"],zl={key:3},Tl={id:"colorScaleHorizontal",x1:"0%",x2:"100%",y1:"0%",y2:"0%"},Ml=["stop-color"],Sl=["stop-color"],Ol=["x","y","width","height","rx"],Nl=["x","y","font-size","fill"],Il=["x","y","font-size","fill"],Vl=["stroke","x1","x2","y1","y2"],Fl=["fill","d"],Pl={key:5,class:"vue-data-ui-watermark"},Bl={key:6,ref:"source",dir:"auto"},Rl={class:"vue-ui-data-table"},jl={key:0},El=["data-cell"],Gl={style:{display:"flex","align-items":"center",gap:"5px","justify-content":"flex-end",width:"100%","padding-right":"3px"}},Hl=["data-cell"],Yl={style:{display:"flex","align-items":"center",gap:"5px","justify-content":"flex-end",width:"100%","padding-right":"3px"}},Wl={__name:"vue-ui-heatmap",props:{config:{type:Object,default(){return{}}},dataset:{type:Array,default(){return[]}}},setup(Y,{expose:Le}){const v=Y;Ie(l=>({"309b083a":l.tdo}));const{vue_ui_heatmap:Ae}=Ve(),W=h(()=>!!v.dataset&&v.dataset.length);Fe(v.dataset)&&we({componentName:"VueUiHeatmap",type:"dataset"});const V=p(oe()),X=p(null),Ce=p(null),Z=p(!1),D=p(""),ne=p(void 0),F=p(null),ve=p(0),re=p(null),_e=p(null),de=p(!1),ye=p(0),e=h({get:()=>ce(),set:l=>l});function ce(){const l=$e({userConfig:v.config,defaultConfig:Ae});let t={};return l.theme?t={...$e({userConfig:Pe.vue_ui_heatmap[l.theme]||v.config,defaultConfig:l})}:t=l,v.config&&Be(v.config,"style.layout.dataLabels.xAxis.showOnlyAtModulo")?t.style.layout.dataLabels.xAxis.showOnlyAtModulo=v.config.style.layout.dataLabels.xAxis.showOnlyAtModulo:t.style.layout.dataLabels.xAxis.showOnlyAtModulo=null,t}ke(()=>v.config,l=>{e.value=ce(),Q(),ye.value+=1},{deep:!0}),ke(()=>v.dataset,()=>{Q()},{deep:!0});const{isPrinting:he,isImaging:pe,generatePdf:ge,generateImage:fe}=Ze({elementId:`heatmap__${V.value}`,fileName:e.value.style.title.text||"vue-ui-heatmap"}),ze=h(()=>e.value.userOptions.show&&!e.value.style.title.text),k=p({showTable:e.value.table.show,showTooltip:e.value.style.tooltip.show}),Te=h(()=>e.value.table.responsiveBreakpoint);function Me(){new ResizeObserver(l=>{l.forEach(t=>{de.value=t.contentRect.width<Te.value})}).observe(re.value)}Re(Q);function Q(){Me()}const N=h(()=>Math.max(...v.dataset.flatMap(l=>(l.values||[]).length))),$=h(()=>{const l=e.value.style.layout.padding.top+e.value.style.layout.padding.bottom+v.dataset.length*e.value.style.layout.cells.height,t=e.value.style.layout.padding.left+e.value.style.layout.padding.right+(N.value<v.dataset.length?v.dataset.length:N.value)*e.value.style.layout.cells.height,s=l+(e.value.style.legend.show?e.value.style.legend.position==="right"?0:e.value.style.layout.cells.height*2:0);return{height:l,heightWithLegend:s,width:t}}),U=h(()=>e.value.style.legend.position),a=h(()=>({top:e.value.style.layout.padding.top,left:e.value.style.layout.padding.left,right:$.value.width-e.value.style.layout.padding.right-(U.value==="right"&&e.value.style.legend.show?92:0),bottom:$.value.height-e.value.style.layout.padding.bottom,height:$.value.height-e.value.style.layout.padding.top-e.value.style.layout.padding.bottom,width:$.value.width-e.value.style.layout.padding.right-e.value.style.layout.padding.left-(U.value==="right"&&e.value.style.legend.show?92:0)})),ee=h(()=>d.value.height*v.dataset.length),x=h(()=>Math.max(...v.dataset.flatMap(l=>l.values))),T=h(()=>Math.min(...v.dataset.flatMap(l=>l.values))),M=h(()=>{const l=v.dataset.flatMap(t=>t.values);return l.reduce((t,s)=>t+s,0)/l.length}),d=h(()=>({width:a.value.width/N.value,height:a.value.width/(N.value<v.dataset.length?v.dataset.length:N.value)})),L=h(()=>{const l=e.value.style.layout.dataLabels.yAxis.values.length?e.value.style.layout.dataLabels.yAxis.values:v.dataset.map(s=>s.name),t=e.value.style.layout.dataLabels.xAxis.values;return{yLabels:l,xLabels:t.slice(0,N.value)}}),q=h(()=>(v.dataset.forEach((l,t)=>{je({datasetObject:l,requiredAttributes:["values"]}).forEach(s=>{we({componentName:"VueUiHeatmap",type:"datasetSerieAttribute",property:"values",index:t})})}),v.dataset.map(l=>({...l,temperatures:(l.values||[]).map((t,s)=>t>=M.value?{side:"up",color:se(e.value.style.layout.cells.colors.cold,e.value.style.layout.cells.colors.hot,T.value,x.value,t),ratio:Math.abs(Math.abs(t-M.value)/Math.abs(x.value-M.value))>1?1:Math.abs(Math.abs(t-M.value)/Math.abs(x.value-M.value)),value:t,yAxisName:l.name,xAxisName:L.value.xLabels[s],id:`vue-data-ui-heatmap-cell-${oe()}`}:{side:"down",ratio:Math.abs(1-Math.abs(t)/Math.abs(M.value))>1?1:Math.abs(1-Math.abs(t)/Math.abs(M.value)),color:se(e.value.style.layout.cells.colors.cold,e.value.style.layout.cells.colors.hot,T.value,x.value,t),value:t,yAxisName:l.name,xAxisName:L.value.xLabels[s],id:`vue-data-ui-heatmap-cell-${oe()}`})})))),A=p(null),P=p(null);function Se(l,t,s,r){F.value={x:s,y:r};const{value:y,yAxisName:g,xAxisName:C,id:Ne}=l;ne.value=Ne,A.value=y,P.value={datapoint:l,seriesIndex:t,series:q.value,config:e.value},Z.value=!0;let te="";const ae=e.value.style.tooltip.customFormat;Xe(ae)&&Ue(()=>ae({datapoint:l,seriesIndex:t,series:q.value,config:e.value}))?D.value=ae({datapoint:l,seriesIndex:t,series:q.value,config:e.value}):(te+=`<div>${g} ${C?`${C}`:""}</div>`,te+=`<div style="margin-top:6px;padding-top:6px;border-top:1px solid ${e.value.style.tooltip.borderColor};font-weight:bold;display:flex;flex-direction:row;gap:12px;align-items:center;justify-content:center"><span style="color:${se(e.value.style.layout.cells.colors.cold,e.value.style.layout.cells.colors.hot,T.value,x.value,y)}">⬤</span><span>${isNaN(y)?"-":I(e.value.style.layout.cells.value.formatter,y,O({p:e.value.style.layout.dataLabels.prefix,v:y,s:e.value.style.layout.dataLabels.suffix,r:e.value.style.tooltip.roundingValue}),{datapoint:l,seriesIndex:t})}</span></div>`,D.value=`<div style="font-size:${e.value.style.tooltip.fontSize}px">${te}</div>`)}const B=h(()=>a.value.top+ee.value*(1-A.value/x.value)),R=h(()=>a.value.left+($.value.width-a.value.left-e.value.style.layout.padding.right)*(A.value/x.value));h(()=>{const l=v.dataset.map(s=>({name:s.name})),t=v.dataset.map(s=>s.values);return{head:l,body:t}});function be(){Ee(()=>{const l=["",...v.dataset.map((y,g)=>y.name)],t=[];for(let y=0;y<L.value.xLabels.length;y+=1){const g=[L.value.xLabels[y]];for(let C=0;C<v.dataset.length;C+=1)g.push([v.dataset[C].values[y]]);t.push(g)}const s=[[e.value.style.title.text],[e.value.style.title.subtitle.text],[[""],[""],[""]]].concat([l]).concat(t),r=Ge(s);He({csvContent:r,title:e.value.style.title.text||"vue-ui-heatmap"})})}const j=p(!1);function Oe(l){j.value=l,ve.value+=1}function xe(){k.value.showTable=!k.value.showTable}function me(){k.value.showTooltip=!k.value.showTooltip}const K=p(!1);function le(){K.value=!K.value}return Le({generatePdf:ge,generateCsv:be,generateImage:fe,toggleTable:xe,toggleTooltip:me,toggleAnnotator:le}),(l,t)=>(o(),u("div",{ref_key:"heatmapChart",ref:X,class:J(`vue-ui-heatmap ${j.value?"vue-data-ui-wrapper-fullscreen":""}`),style:w(`font-family:${e.value.style.fontFamily};width:100%; text-align:center;background:${e.value.style.backgroundColor}`),id:`heatmap__${V.value}`},[e.value.userOptions.buttons.annotator?(o(),E(sl,{key:0,parent:X.value,backgroundColor:e.value.style.backgroundColor,color:e.value.style.color,active:K.value,onClose:le},null,8,["parent","backgroundColor","color","active"])):c("",!0),ze.value?(o(),u("div",{key:1,ref_key:"noTitle",ref:_e,class:"vue-data-ui-no-title-space",style:"height:36px; width: 100%;background:transparent"},null,512)):c("",!0),e.value.style.title.text?(o(),u("div",il,[(o(),E(Je,{key:`title_${ye.value}`,config:{title:{cy:"heatmap-div-title",...e.value.style.title},subtitle:{cy:"heatmap-div-subtitle",...e.value.style.title.subtitle}}},null,8,["config"]))])):c("",!0),e.value.userOptions.show&&W.value?(o(),E(De,{ref_key:"details",ref:Ce,key:`user_options_${ve.value}`,backgroundColor:e.value.style.backgroundColor,color:e.value.style.color,isImaging:n(pe),isPrinting:n(he),uid:V.value,hasTooltip:e.value.userOptions.buttons.tooltip&&e.value.style.tooltip.show,hasPdf:e.value.userOptions.buttons.pdf,hasImg:e.value.userOptions.buttons.img,hasXls:e.value.userOptions.buttons.csv,hasTable:e.value.userOptions.buttons.table,hasFullscreen:e.value.userOptions.buttons.fullscreen,isFullscreen:j.value,isTooltip:k.value.showTooltip,titles:{...e.value.userOptions.buttonTitles},chartElement:X.value,position:e.value.userOptions.position,hasAnnotator:e.value.userOptions.buttons.annotator,isAnnotation:K.value,onToggleFullscreen:Oe,onGeneratePdf:n(ge),onGenerateCsv:be,onGenerateImage:n(fe),onToggleTable:xe,onToggleTooltip:me,onToggleAnnotator:le},Ye({_:2},[l.$slots.optionTooltip?{name:"optionTooltip",fn:m(()=>[f(l.$slots,"optionTooltip",{},void 0,!0)]),key:"0"}:void 0,l.$slots.optionPdf?{name:"optionPdf",fn:m(()=>[f(l.$slots,"optionPdf",{},void 0,!0)]),key:"1"}:void 0,l.$slots.optionCsv?{name:"optionCsv",fn:m(()=>[f(l.$slots,"optionCsv",{},void 0,!0)]),key:"2"}:void 0,l.$slots.optionImg?{name:"optionImg",fn:m(()=>[f(l.$slots,"optionImg",{},void 0,!0)]),key:"3"}:void 0,l.$slots.optionTable?{name:"optionTable",fn:m(()=>[f(l.$slots,"optionTable",{},void 0,!0)]),key:"4"}:void 0,l.$slots.optionFullscreen?{name:"optionFullscreen",fn:m(({toggleFullscreen:s,isFullscreen:r})=>[f(l.$slots,"optionFullscreen",G(H({toggleFullscreen:s,isFullscreen:r})),void 0,!0)]),key:"5"}:void 0,l.$slots.optionAnnotator?{name:"optionAnnotator",fn:m(({toggleAnnotator:s,isAnnotator:r})=>[f(l.$slots,"optionAnnotator",G(H({toggleAnnotator:s,isAnnotator:r})),void 0,!0)]),key:"6"}:void 0]),1032,["backgroundColor","color","isImaging","isPrinting","uid","hasTooltip","hasPdf","hasImg","hasXls","hasTable","hasFullscreen","isFullscreen","isTooltip","titles","chartElement","position","hasAnnotator","isAnnotation","onGeneratePdf","onGenerateImage"])):c("",!0),W.value?(o(),u("svg",{key:4,xmlns:n(We),class:J({"vue-data-ui-fullscreen--on":j.value,"vue-data-ui-fulscreen--off":!j.value}),viewBox:`0 0 ${$.value.width} ${$.value.heightWithLegend}`,style:w(`max-width:100%;overflow:visible;background:transparent;color:${e.value.style.color}`)},[ue(ol),(o(!0),u(_,null,S(q.value,(s,r)=>(o(),u("g",null,[(o(!0),u(_,null,S(s.temperatures,(y,g)=>(o(),u("g",null,[i("rect",{x:a.value.left+d.value.width*g+e.value.style.layout.cells.spacing/2,y:a.value.top+d.value.height*r+e.value.style.layout.cells.spacing/2,width:d.value.width-e.value.style.layout.cells.spacing,height:d.value.height-e.value.style.layout.cells.spacing,fill:e.value.style.layout.cells.colors.underlayer,stroke:e.value.style.backgroundColor,"stroke-width":e.value.style.layout.cells.spacing},null,8,vl),i("rect",{x:a.value.left+d.value.width*g+e.value.style.layout.cells.spacing/2,y:a.value.top+d.value.height*r+e.value.style.layout.cells.spacing/2,width:d.value.width-e.value.style.layout.cells.spacing,height:d.value.height-e.value.style.layout.cells.spacing,fill:y.color,stroke:e.value.style.backgroundColor,"stroke-width":e.value.style.layout.cells.spacing},null,8,rl),e.value.style.layout.cells.value.show?(o(),u("text",{key:0,"text-anchor":"middle","font-size":e.value.style.layout.cells.value.fontSize,"font-weight":e.value.style.layout.cells.value.bold?"bold":"normal",fill:n(ie)(y.color),x:a.value.left+d.value.width*g+d.value.width/2,y:a.value.top+d.value.height*r+d.value.height/2+e.value.style.layout.cells.value.fontSize/3},b(n(I)(e.value.style.layout.cells.value.formatter,y.value,n(O)({p:e.value.style.layout.dataLabels.prefix,v:y.value,s:e.value.style.layout.dataLabels.suffix,r:e.value.style.layout.cells.value.roundingValue}),{datapoint:y})),9,dl)):c("",!0)]))),256)),(o(!0),u(_,null,S(s.temperatures,(y,g)=>(o(),u("g",null,[i("rect",{x:a.value.left+d.value.width*g,y:a.value.top+d.value.height*r,width:d.value.width,height:d.value.height,fill:"transparent",stroke:"none",onMouseover:C=>Se(y,r,a.value.left+d.value.width*g,a.value.top+d.value.height*r),onMouseout:t[0]||(t[0]=C=>{Z.value=!1,ne.value=void 0,A.value=null,F.value=null})},null,40,yl)]))),256)),e.value.style.layout.dataLabels.yAxis.show?(o(),u("g",cl,[i("text",{"font-size":e.value.style.layout.dataLabels.yAxis.fontSize,fill:e.value.style.layout.dataLabels.yAxis.color,x:a.value.left+e.value.style.layout.dataLabels.yAxis.offsetX-6,y:a.value.top+d.value.height*r+d.value.height/2+e.value.style.layout.dataLabels.yAxis.fontSize/3+e.value.style.layout.dataLabels.yAxis.offsetY,"text-anchor":"end","font-weight":e.value.style.layout.dataLabels.yAxis.bold?"bold":"normal"},b(L.value.yLabels[r]),9,hl)])):c("",!0)]))),256)),e.value.style.layout.dataLabels.xAxis.show?(o(),u("g",pl,[(o(!0),u(_,null,S(L.value.xLabels,(s,r)=>(o(),u(_,null,[!e.value.style.layout.dataLabels.xAxis.showOnlyAtModulo||e.value.style.layout.dataLabels.xAxis.showOnlyAtModulo&&r%e.value.style.layout.dataLabels.xAxis.showOnlyAtModulo===0?(o(),u("text",{key:0,"text-anchor":e.value.style.layout.dataLabels.xAxis.rotation===0?"middle":e.value.style.layout.dataLabels.xAxis.rotation<0?"start":"end","font-size":e.value.style.layout.dataLabels.xAxis.fontSize,fill:e.value.style.layout.dataLabels.xAxis.color,"font-weight":e.value.style.layout.dataLabels.xAxis.bold?"bold":"normal",transform:`translate(${a.value.left+d.value.width/2+a.value.width/L.value.xLabels.length*r+e.value.style.layout.dataLabels.xAxis.offsetX}, ${a.value.top+e.value.style.layout.dataLabels.xAxis.offsetY-6}), rotate(${e.value.style.layout.dataLabels.xAxis.rotation})`},b(s),9,gl)):c("",!0)],64))),256))])):c("",!0),F.value?(o(),u("g",fl,[i("rect",{style:{"pointer-events":"none"},x:F.value.x-e.value.style.layout.cells.selected.border/2+e.value.style.layout.cells.spacing,y:F.value.y-e.value.style.layout.cells.selected.border/2+e.value.style.layout.cells.spacing,width:d.value.width-e.value.style.layout.cells.spacing+e.value.style.layout.cells.selected.border-e.value.style.layout.cells.spacing,height:d.value.height-e.value.style.layout.cells.spacing+e.value.style.layout.cells.selected.border-e.value.style.layout.cells.spacing,fill:"transparent",stroke:e.value.style.layout.cells.selected.color,"stroke-width":e.value.style.layout.cells.selected.border,rx:1},null,8,bl)])):c("",!0),e.value.style.legend.show&&U.value==="right"?(o(),u("g",xl,[i("defs",null,[i("linearGradient",{id:`colorScaleVertical_${V.value}`,x2:"0%",y2:"100%"},[i("stop",{offset:"0%","stop-color":e.value.style.layout.cells.colors.hot},null,8,wl),i("stop",{offset:"100%","stop-color":e.value.style.layout.cells.colors.cold},null,8,kl)],8,ml)]),i("text",{x:a.value.right+36+18,y:a.value.top-e.value.style.legend.fontSize*1.5,"text-anchor":"middle","font-size":e.value.style.legend.fontSize*2,fill:e.value.style.legend.color},b(n(I)(e.value.style.layout.cells.value.formatter,n(z)(x.value),n(O)({p:e.value.style.layout.dataLabels.prefix,v:n(z)(x.value),s:e.value.style.layout.dataLabels.suffix,r:e.value.style.legend.roundingValue}))),9,$l),i("rect",{x:a.value.right+36,y:a.value.top,width:36,height:ee.value,rx:e.value.style.legend.scaleBorderRadius,fill:`url(#colorScaleVertical_${V.value})`},null,8,Ll),i("text",{x:a.value.right+36+18,y:a.value.top+ee.value+e.value.style.legend.fontSize*2.5,"text-anchor":"middle","font-size":e.value.style.legend.fontSize*2,fill:e.value.style.legend.color},b(n(I)(e.value.style.layout.cells.value.formatter,n(z)(T.value),n(O)({p:e.value.style.layout.dataLabels.prefix,v:n(z)(T.value),s:e.value.style.layout.dataLabels.suffix,r:e.value.style.legend.roundingValue}))),9,Al),A.value!==null?(o(),u("line",{key:0,stroke:n(ie)(P.value.datapoint.color),"stroke-width":"2",x1:a.value.right+36,x2:a.value.right+72,y1:B.value,y2:B.value},null,8,Cl)):c("",!0),A.value!==null?(o(),u("path",{key:1,fill:e.value.style.color,stroke:"none",d:`M ${a.value.right+36},${B.value} ${a.value.right+26},${B.value-8} ${a.value.right+26},${B.value+8}z`},null,8,_l)):c("",!0)])):c("",!0),e.value.style.legend.show&&U.value==="bottom"?(o(),u("g",zl,[i("defs",null,[i("linearGradient",Tl,[i("stop",{offset:"0%","stop-color":e.value.style.layout.cells.colors.cold},null,8,Ml),i("stop",{offset:"100%","stop-color":e.value.style.layout.cells.colors.hot},null,8,Sl)])]),i("rect",{x:a.value.left,y:a.value.bottom+e.value.style.layout.cells.height,width:$.value.width-a.value.left-e.value.style.layout.padding.right,height:e.value.style.layout.cells.height,rx:e.value.style.legend.scaleBorderRadius>e.value.style.layout.cells.height/2?e.value.style.layout.cells.height/2:e.value.style.legend.scaleBorderRadius,fill:"url(#colorScaleHorizontal)"},null,8,Ol),i("text",{x:a.value.left,y:a.value.bottom+e.value.style.layout.cells.height*2+e.value.style.legend.fontSize*2,"text-anchor":"start","font-size":e.value.style.legend.fontSize*2,fill:e.value.style.legend.color},b(n(I)(e.value.style.layout.cells.value.formatter,n(z)(T.value),n(O)({p:e.value.style.layout.dataLabels.prefix,v:n(z)(T.value),s:e.value.style.layout.dataLabels.suffix,r:e.value.style.legend.roundingValue}))),9,Nl),i("text",{x:a.value.right,y:a.value.bottom+e.value.style.layout.cells.height*2+e.value.style.legend.fontSize*2,"text-anchor":"end","font-size":e.value.style.legend.fontSize*2,fill:e.value.style.legend.color},b(n(I)(e.value.style.layout.cells.value.formatter,n(z)(x.value),n(O)({p:e.value.style.layout.dataLabels.prefix,v:n(z)(x.value),s:e.value.style.layout.dataLabels.suffix,r:e.value.style.legend.roundingValue}))),9,Il),A.value!==null?(o(),u("line",{key:0,stroke:n(ie)(P.value.datapoint.color),"stroke-width":"2",x1:R.value,x2:R.value,y1:a.value.bottom+e.value.style.layout.cells.height,y2:a.value.bottom+e.value.style.layout.cells.height*2},null,8,Vl)):c("",!0),A.value!==null?(o(),u("path",{key:1,fill:e.value.style.color,stroke:"none",d:`M ${R.value},${a.value.bottom+e.value.style.layout.cells.height} ${R.value-12},${a.value.bottom+e.value.style.layout.cells.height-20} ${R.value+12},${a.value.bottom+e.value.style.layout.cells.height-20}z`},null,8,Fl)):c("",!0)])):c("",!0),f(l.$slots,"svg",{svg:$.value},void 0,!0)],14,nl)):c("",!0),l.$slots.watermark?(o(),u("div",Pl,[f(l.$slots,"watermark",G(H({isPrinting:n(he)||n(pe)})),void 0,!0)])):c("",!0),l.$slots.source?(o(),u("div",Bl,[f(l.$slots,"source",{},void 0,!0)],512)):c("",!0),W.value?c("",!0):(o(),E(el,{key:7,config:{type:"heatmap",style:{backgroundColor:e.value.style.backgroundColor,heatmap:{color:"#CCCCCC"}}}},null,8,["config"])),ue(Qe,{show:k.value.showTooltip&&Z.value,backgroundColor:e.value.style.tooltip.backgroundColor,color:e.value.style.tooltip.color,borderRadius:e.value.style.tooltip.borderRadius,borderColor:e.value.style.tooltip.borderColor,borderWidth:e.value.style.tooltip.borderWidth,fontSize:e.value.style.tooltip.fontSize,backgroundOpacity:e.value.style.tooltip.backgroundOpacity,position:e.value.style.tooltip.position,offsetY:e.value.style.tooltip.offsetY,parent:X.value,content:D.value,isCustom:e.value.style.tooltip.customFormat&&typeof e.value.style.tooltip.customFormat=="function"},{"tooltip-before":m(()=>[f(l.$slots,"tooltip-before",G(H({...P.value})),void 0,!0)]),"tooltip-after":m(()=>[f(l.$slots,"tooltip-after",G(H({...P.value})),void 0,!0)]),_:3},8,["show","backgroundColor","color","borderRadius","borderColor","borderWidth","fontSize","backgroundOpacity","position","offsetY","parent","content","isCustom"]),W.value?(o(),E(al,{key:8,hideDetails:"",config:{open:k.value.showTable,maxHeight:1e4,body:{backgroundColor:e.value.style.backgroundColor,color:e.value.style.color},head:{backgroundColor:e.value.style.backgroundColor,color:e.value.style.color}}},{content:m(()=>[i("div",{ref_key:"tableContainer",ref:re,class:"vue-ui-heatmap-table"},[i("div",{style:"width:100%;overflow-x:auto;padding-top:36px;position:relative",class:J({"vue-ui-responsive":de.value})},[i("div",{role:"button",tabindex:"0",style:w(`width:32px; position: absolute; top: 0; left:4px; padding: 0 0px; display: flex; align-items:center;justify-content:center;height: 36px; width: 32px; cursor:pointer; background:${e.value.table.th.backgroundColor};`),onClick:t[1]||(t[1]=s=>k.value.showTable=!1),onKeypress:t[2]||(t[2]=qe(s=>k.value.showTable=!1,["enter"]))},[ue(tl,{name:"close",stroke:e.value.table.th.color,"stroke-width":2},null,8,["stroke"])],36),i("table",Rl,[i("caption",{style:w(`backgroundColor:${e.value.table.th.backgroundColor};color:${e.value.table.th.color};outline:${e.value.table.th.outline}`)},[Ke(b(e.value.style.title.text)+" ",1),e.value.style.title.subtitle.text?(o(),u("span",jl,b(e.value.style.title.subtitle.text),1)):c("",!0)],4),i("thead",null,[i("tr",{role:"row",style:w(`background:${e.value.table.th.backgroundColor};color:${e.value.table.th.color}`)},[i("th",{style:w(`outline:${e.value.table.th.outline};padding-right:6px`)},null,4),(o(!0),u(_,null,S(Y.dataset,(s,r)=>(o(),u("th",{align:"right",style:w(`outline:${e.value.table.th.outline};padding-right:6px`)},b(s.name),5))),256))],4)]),i("tbody",null,[(o(!0),u(_,null,S(L.value.xLabels,(s,r)=>(o(),u("tr",{role:"row",class:J({"vue-ui-data-table__tbody__row":!0,"vue-ui-data-table__tbody__row-even":r%2===0,"vue-ui-data-table__tbody__row-odd":r%2!==0}),style:w(`background:${e.value.table.td.backgroundColor};color:${e.value.table.td.color}`)},[i("td",{"data-cell":e.value.table.colNames.xAxis,class:"vue-ui-data-table__tbody__td",style:w(`outline:${e.value.table.td.outline}`)},[i("div",Gl,b(s),1)],12,El),(o(!0),u(_,null,S(Y.dataset,(y,g)=>(o(),u("td",{class:"vue-ui-data-table__tbody__td","data-cell":Y.dataset[g].name,style:w(`outline:${e.value.table.td.outline}`)},[i("div",Yl,b(isNaN(y.values[r])?"-":n(O)({p:e.value.style.layout.dataLabels.prefix,v:y.values[r],s:e.value.style.layout.dataLabels.suffix,r:e.value.table.td.roundingValue})),1)],12,Hl))),256))],6))),256))])])],2)],512)]),_:1},8,["config"])):c("",!0)],14,ul))}},tt=ll(Wl,[["__scopeId","data-v-569c98af"]]);export{tt as default};
