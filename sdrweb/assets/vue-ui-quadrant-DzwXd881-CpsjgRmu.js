import{z as Ze,c as f,r as y,D as Be,R as el,A as ne,H as ll,w as He,B as al,C as I,b as tl,ac as ol,E as ul,Q as sl,ay as rl,U as N,G as V,K as il,V as nl,N as vl,o as u,a as i,i as B,L as d,j as cl,l as x,m as L,p as D,q as J,u as h,M as z,n as T,J as dl,f as ve,e as v,P as S,k as A,O as te,a1 as yl,h as $,az as hl,a2 as bl,a3 as pl,ag as gl,g as fl}from"./index-DIL_pLmJ.js";import{O as ml,k as xl}from"./useResponsive-CoxXLe23-CoJWR8uA.js";import{x as $l}from"./Title-DyD-mBPN-CHqvPevn.js";import{r as kl,a as Ll}from"./usePrinter-CLcxWv-4-CCDxi2Mm.js";import{E as wl}from"./Tooltip-qod9o4Vz-Dsvb9C65.js";import{$ as Re}from"./Shape-DYFnvMNd-DLTmTYE_.js";import{C as ql}from"./Legend-BorSDXCj-Cb5_Z7Gi.js";import{W as zl}from"./DataTable-D8fcY_sJ-By-GDpuN.js";import Sl from"./vue-ui-skeleton-Dgzcn3FL-BNvPX0g2.js";import Cl from"./vue-ui-accordion-m8rut1bP-DBR24duA.js";import{c as Oe}from"./useNestedProp-BxrcwmTZ-1Bk3HyKT.js";import{p as Wl}from"./PackageVersion-mSsg8dpu-BxnQ4_xj.js";import{S as Tl}from"./PenAndPaper-Dk41ZrJ0-D5uht0-m.js";import{s as Xl}from"./vue-data-ui-DF7u-eZV-hcmE5A1B.js";const Yl=["id"],Al=["xmlns","viewBox","id"],_l=["id"],Ml=["stop-color"],Nl=["stop-color"],Vl={key:0},Bl=["fill","x","y","height","width","stroke-width","stroke","rx"],Hl=["x1","y1","x2","y2","stroke","stroke-width"],Rl=["x1","y1","x2","y2","stroke","stroke-width"],Ol={key:1},Il=["points","fill"],Pl=["points","fill"],Fl=["points","fill"],El=["points","fill"],Gl={key:2},Ul=["y","fill","font-size"],Dl=["x","y","fill","font-size"],Jl=["x","y","fill","font-size"],Ql=["y","fill","font-size"],jl={key:3},Kl=["x","y","font-size","fill"],Zl=["x","y","font-size","fill"],ea=["x","y","font-size","fill"],la=["id","font-size","transform","fill"],aa=["id","font-size","transform","fill"],ta=["id","font-size","transform","fill"],oa={key:4},ua=["fill","points"],sa=["x","y","width","height"],ra=["x","y","width","height"],ia=["x","y","width","height"],na=["x","y","width","height"],va={key:0,style:{"pointer-events":"none"}},ca=["x","y","font-size","fill"],da={key:0},ya=["x","y","onMouseover","onClick"],ha={key:7,class:"vue-ui-dna"},ba=["points","fill"],pa=["points","fill"],ga=["points","fill"],fa=["points","fill"],ma={key:8},xa=["x","y","font-size","fill","font-weight"],$a={key:9},ka=["x","y","fill"],La=["x","y","fill"],wa=["x","y","fill"],qa=["x","y","fill"],za=["stroke","d"],Sa=["stroke","d"],Ca={key:5,class:"vue-data-ui-watermark"},Wa=["onClick"],Ta={key:0,height:"14",width:"14",viewBox:"0 0 20 20"},Xa=["innerHTML"],Ya={__name:"vue-ui-quadrant",props:{config:{type:Object,default(){return{}}},dataset:{type:Array,default(){return[]}}},emits:["selectPlot","selectSide","selectLegend"],setup(Ie,{expose:Pe,emit:oe}){const X=Ie,{vue_ui_quadrant:Fe}=Ze(),Q=f(()=>!!X.dataset&&X.dataset.length),W=y(Be()),Ee=y(null),j=y(!1),ue=y(""),ce=y(0),g=y(!1),R=y(null),de=y(null),ye=y(null),he=y(null),be=y(null),pe=y(0),ge=y(0),fe=y(0),l=f({get:()=>me(),set:a=>a});function me(){const a=Oe({userConfig:X.config,defaultConfig:Fe});return a.theme?{...Oe({userConfig:ll.vue_ui_quadrant[a.theme]||X.config,defaultConfig:a}),customPalette:el[a.theme]||ne}:a}He(()=>X.config,a=>{l.value=me(),xe(),pe.value+=1,ge.value+=1,fe.value+=1},{deep:!0});const K=y(null);function xe(){if(al(X.dataset)?I({componentName:"VueUiQuadrant",type:"dataset"}):X.dataset.forEach((a,o)=>{[null,void 0].includes(a.name)&&I({componentName:"VueUiQuadrant",type:"datasetSerieAttribute",property:"name",index:o}),[null,void 0].includes(a.series)?I({componentName:"VueUiQuadrant",type:"datasetSerieAttribute",property:"series",index:o}):a.series.forEach((t,s)=>{[null,void 0].includes(t.name)&&I({componentName:"VueUiQuadrant",type:"datasetSerieAttribute",property:"name",key:"series",index:s})})}),l.value.responsive){const a=ml(()=>{const{width:o,height:t}=xl({chart:R.value,title:l.value.style.chart.title.text?de.value:null,legend:l.value.style.chart.legend.show?ye.value:null,source:he.value,noTitle:be.value});m.value=48,e.value.height=t,e.value.usableHeight=t-m.value*2,e.value.width=o,e.value.usableWidth=o-m.value*2,e.value.top=m.value,e.value.left=m.value,e.value.right=o-m.value,e.value.bottom=t-m.value,e.value.centerX=o/2,e.value.centerY=t-t/2,e.value.padding=m.value});K.value=new ResizeObserver(a),K.value.observe(R.value.parentNode)}}tl(()=>{xe()}),ol(()=>{K.value&&K.value.disconnect()});const{isPrinting:$e,isImaging:ke,generatePdf:Le,generateImage:we}=kl({elementId:`vue-ui-quadrant_${W.value}`,fileName:l.value.style.chart.title.text||"vue-ui-quadrant"}),Ge=f(()=>l.value.userOptions.show&&!l.value.style.chart.title.text),qe=f(()=>ul(l.value.customPalette)),w=y({plotLabels:{show:l.value.style.chart.layout.labels.plotLabels.show},showTable:l.value.table.show,showTooltip:l.value.style.chart.tooltip.show}),m=y(48),e=y({height:l.value.style.chart.height,usableHeight:l.value.style.chart.height-m.value*2,width:l.value.style.chart.width,usableWidth:l.value.style.chart.width-m.value*2,top:m.value,left:m.value,right:l.value.style.chart.width-m.value,bottom:l.value.style.chart.height-m.value,centerX:l.value.style.chart.width/2,centerY:l.value.style.chart.height-l.value.style.chart.height/2,padding:m.value}),c=y({...JSON.parse(JSON.stringify(e.value)),startX:0,startY:0});He(()=>e.value,a=>{a&&(c.value={...JSON.parse(JSON.stringify(e.value)),startX:0,startY:0})},{deep:!0});const n=y(null),H=f(()=>{switch(n.value){case"TL":return{x:c.value.startX+c.value.width/2,y:c.value.height,text:l.value.style.chart.layout.labels.quadrantLabels.tl.text||"Top Left",fontSize:l.value.style.chart.layout.labels.quadrantLabels.tl.fontSize,fill:l.value.style.chart.layout.labels.quadrantLabels.tl.color,bold:l.value.style.chart.layout.labels.quadrantLabels.tl.bold};case"TR":return{x:c.value.startX+c.value.width/2,y:c.value.height,text:l.value.style.chart.layout.labels.quadrantLabels.tr.text||"Top Right",fontSize:l.value.style.chart.layout.labels.quadrantLabels.tr.fontSize,fill:l.value.style.chart.layout.labels.quadrantLabels.tr.color,bold:l.value.style.chart.layout.labels.quadrantLabels.tr.bold};case"BR":return{x:c.value.startX+c.value.width/2,y:c.value.height*1.678,text:l.value.style.chart.layout.labels.quadrantLabels.br.text||"Bottom Right",fontSize:l.value.style.chart.layout.labels.quadrantLabels.br.fontSize,fill:l.value.style.chart.layout.labels.quadrantLabels.br.color,bold:l.value.style.chart.layout.labels.quadrantLabels.br.bold};case"BL":return{x:c.value.startX+c.value.width/2,y:c.value.height*1.678,text:l.value.style.chart.layout.labels.quadrantLabels.bl.text||"Bottom Left",fontSize:l.value.style.chart.layout.labels.quadrantLabels.bl.fontSize,fill:l.value.style.chart.layout.labels.quadrantLabels.bl.color,bold:l.value.style.chart.layout.labels.quadrantLabels.bl.bold};default:return{x:0,y:0,text:"",fontSize:0,fill:"none",bold:!1}}}),Ue=y(null),Z=y(!1);function P({targetX:a,targetY:o,targetW:t,targetH:s,side:r}){n.value&&G(r);const b={x:a-c.value.startX,y:o-c.value.startY,w:t-c.value.width,h:s-c.value.height},p=l.value.zoomAnimationFrames;let Y=0;function Ve(){Z.value=!0,c.value.startX+=b.x/p,c.value.startY+=b.y/p,c.value.width+=b.w/p,c.value.height+=b.h/p,Y+=1,Y<p?Ue.value=requestAnimationFrame(Ve):Z.value=!1}Ve()}function _(a){if(!Z.value)if(g.value&&n.value===a)P({targetX:0,targetY:0,targetW:e.value.width,targetH:e.value.height}),n.value=null,g.value=!1;else{switch(n.value=a,a){case"TL":P({targetX:0,targetY:0,targetW:e.value.width/2+e.value.left,targetH:e.value.height/2+e.value.top,side:"tl"});break;case"TR":P({targetX:e.value.width/2-e.value.left,targetY:0,targetW:e.value.width/2+e.value.left,targetH:e.value.height/2+e.value.top,side:"tr"});break;case"BR":P({targetX:e.value.width/2-e.value.left,targetY:e.value.height/2-e.value.top,targetW:e.value.width/2+e.value.left,targetH:e.value.height/2+e.value.top,side:"br"});break;case"BL":P({targetX:0,targetY:e.value.height/2-e.value.top,targetW:e.value.width/2+e.value.left,targetH:e.value.height/2+e.value.top,side:"bl"});break;default:c.value.startX=0,c.value.startY=0,c.value.width=e.value.width,c.value.height=e.value.height;break}g.value=!0}}const De=f(()=>{const a=l.value.style.chart.layout.grid.graduations.steps,o=e.value.usableWidth/(a*2),t=e.value.top,s=[];for(let r=0;r<a;r+=1)s.push({x:e.value.padding+o*r,y:t+e.value.usableHeight*(r/a/2),height:e.value.usableHeight*(1-r/a),width:e.value.usableWidth*(1-r/a),opacity:Math.round((r+1)/a*20)});return s}),q=f(()=>{let a=l.value.style.chart.layout.grid.xAxis.max,o=l.value.style.chart.layout.grid.xAxis.min,t=l.value.style.chart.layout.grid.yAxis.max,s=l.value.style.chart.layout.grid.yAxis.min;return l.value.style.chart.layout.grid.xAxis.auto&&(a=Math.max(...O.value.flatMap(r=>r.series.map(b=>b.x))),o=Math.min(...O.value.flatMap(r=>r.series.map(b=>b.x)))),l.value.style.chart.layout.grid.yAxis.auto&&(t=Math.max(...O.value.flatMap(r=>r.series.map(b=>b.y))),s=Math.min(...O.value.flatMap(r=>r.series.map(b=>b.y)))),{x:{max:a,min:o},y:{max:t,min:s}}}),M=y([]),O=f(()=>X.dataset.map((a,o)=>({...a,series:rl({data:a.series,threshold:l.value.downsample.threshold}),id:`cat_${o}_${W.value}`,color:sl(a.color)||qe.value[o]||ne[o]}))),Je=f(()=>O.value.filter(a=>!M.value.includes(a.id))),Qe=f(()=>O.value.map((a,o)=>({...a,shape:a.shape||"circle",series:a.series.map(t=>({...t,x:ze(t.x),y:Se(t.y),xValue:t.x,yValue:t.y,quadrantSide:We({x:t.x,y:t.y}),categoryName:a.name,shape:a.shape,color:a.color}))}))),C=f(()=>(X.dataset.forEach((a,o)=>{a.series.forEach((t,s)=>{[null,void 0].includes(t.x)&&I({componentName:"VueUiQuadrant",type:"datasetSerieAttribute",property:"x",key:"series",index:s}),[null,void 0].includes(t.y)&&I({componentName:"VueUiQuadrant",type:"datasetSerieAttribute",property:"y",key:"series",index:s})})}),Je.value.map((a,o)=>({...a,shape:a.shape||"circle",color:a.color||qe.value[o]||ne[o],series:a.series.map(t=>({...t,x:ze(t.x),y:Se(t.y),xValue:t.x,yValue:t.y,quadrantSide:We({x:t.x,y:t.y}),categoryName:a.name,shape:a.shape,color:a.color,uid:Be()}))}))));function ze(a){if(a>=0){const o=a/q.value.x.max;return e.value.centerX+e.value.usableWidth/2*o}else{const o=Math.abs(a)/Math.abs(q.value.x.min);return e.value.centerX-e.value.usableWidth/2*o}}function Se(a){if(a>=0){const o=a/q.value.y.max;return e.value.centerY+(1-e.value.usableHeight/2*o)}else{const o=Math.abs(a)/Math.abs(q.value.y.min);return e.value.centerY-(1-e.value.usableHeight/2*o)}}const ee=f(()=>{const a=C.value.flatMap(p=>p.series.map(Y=>({x:Y.xValue,y:Y.yValue,name:Y.name,category:Y.categoryName,quadrantSide:Y.quadrantSide,sideName:l.value.style.chart.layout.labels.quadrantLabels[Y.quadrantSide].text,color:p.color,shape:p.shape}))),o=l.value.style.chart.layout.grid.xAxis.name||"x",t=l.value.style.chart.layout.grid.yAxis.name||"y",s=[l.value.translations.category,l.value.translations.item,o,t,l.value.translations.side],r=a.map(p=>[p.category,p.name,p.x,p.y,p.sideName||p.quadrantSide]),b=a.map(p=>({shape:p.shape,color:p.color}));return{head:s,body:r,itsShapes:b,tableData:a}}),le=f(()=>{const a=ee.value.head,o=ee.value.tableData.map(s=>[{shape:s.shape,color:s.color,name:s.category},s.name,N(l.value.style.chart.layout.labels.plotLabels.x.formatter,s.x,V({v:s.x,r:l.value.style.chart.layout.labels.plotLabels.rounding})),N(l.value.style.chart.layout.labels.plotLabels.y.formatter,s.y,V({v:s.y,r:l.value.style.chart.layout.labels.plotLabels.rounding})),s.sideName||s.quadrantSide]),t={th:{backgroundColor:l.value.table.th.backgroundColor,color:l.value.table.th.color,outline:l.value.table.th.outline},td:{backgroundColor:l.value.table.td.backgroundColor,color:l.value.table.td.color,outline:l.value.table.td.outline},breakpoint:l.value.table.responsiveBreakpoint};return{head:a,body:o,config:t,colNames:a}});function se(a){M.value.includes(a)?M.value=M.value.filter(t=>t!==a):M.value.push(a);const o=Ye();oe("selectLegend",o)}const Ce=f(()=>Qe.value.map(a=>({name:a.name,shape:a.shape,color:a.color,id:a.id,opacity:M.value.includes(a.id)?.5:1,segregate:()=>se(a.id),isSegregated:M.value.includes(a.id)}))),je=f(()=>({cy:"quadrant-div-legend",backgroundColor:l.value.style.chart.legend.backgroundColor,color:l.value.style.chart.legend.color,fontSize:l.value.style.chart.legend.fontSize,paddingBottom:12,fontWeight:l.value.style.chart.legend.bold?"bold":""}));function We(a){switch(!0){case(a.x>=0&&a.y>=0):return"tr";case(a.x>=0&&a.y<0):return"br";case(a.x<0&&a.y<0):return"bl";case(a.x<0&&a.y>=0):return"tl";default:return""}}const F=y(null),E=y(null),re=y(null);function Te(a,o,t){F.value=o.uid,E.value={color:a.color,shape:a.shape},re.value={datapoint:o,seriesIndex:t,series:C.value,config:l.value},j.value=!0;const s=l.value.style.chart.tooltip.customFormat;if(bl(s)&&pl(()=>s({seriesIndex:t,datapoint:o,series:C.value,config:l.value})))ue.value=s({seriesIndex:t,datapoint:o,series:C.value,config:l.value});else{let r="";o.quadrantSide&&(r+=`<div style="color:${l.value.style.chart.layout.labels.quadrantLabels[o.quadrantSide].color};font-weight:${l.value.style.chart.layout.labels.quadrantLabels[o.quadrantSide].bold?"bold":"400"}">${l.value.style.chart.layout.labels.quadrantLabels[o.quadrantSide].text}</div>`),r+=`<div>${a.name}</div>`,r+=`<div style="padding-bottom:6px;border-bottom:1px solid ${l.value.style.chart.tooltip.borderColor};margin-bottom:3px">${o.name}</div>`,r+=`<div>${l.value.style.chart.layout.grid.xAxis.name?l.value.style.chart.layout.grid.xAxis.name:"x"}: <b>${N(l.value.style.chart.layout.labels.plotLabels.x.formatter,o.xValue,V({v:o.xValue,r:l.value.style.chart.tooltip.roundingValue}),{datapoint:o,category:a,categoryIndex:t})}</b></div>`,r+=`<div>${l.value.style.chart.layout.grid.yAxis.name?l.value.style.chart.layout.grid.yAxis.name:"y"}: <b>${N(l.value.style.chart.layout.labels.plotLabels.y.formatter,o.yValue,V({v:o.yValue,r:l.value.style.chart.tooltip.roundingValue}),{datapoint:o,category:a,categoryIndex:t})}</b></div>`,ue.value=`<div style="text-align:left;font-size:${l.value.style.chart.tooltip.fontSize}px">${r}</div>`}}function Xe(a,o){const t={category:a.name,shape:a.shape,itemName:o.name,x:o.xValue,y:o.yValue,quadrantSide:o.quadrantSide,sideName:l.value.style.chart.layout.labels.quadrantLabels[o.quadrantSide].text};oe("selectPlot",t)}function G(a){if(!a)return;const o=C.value.flatMap(s=>s.series.filter(r=>r.quadrantSide===a).map(r=>({category:r.categoryName,itemName:r.name,x:r.xValue,y:r.yValue}))),t={quadrantSide:a,sideName:l.value.style.chart.layout.labels.quadrantLabels[a].text,items:[...o]};oe("selectSide",t)}const k=f(()=>({TL:{tl:{x:e.value.left+e.value.usableWidth/4-20,y:0,fill:l.value.style.chart.layout.labels.quadrantLabels.tl.color},tr:{x:e.value.left+e.value.usableWidth/4,y:0,fill:l.value.style.chart.layout.labels.quadrantLabels.tr.color},br:{x:e.value.left+e.value.usableWidth/4,y:20,fill:l.value.style.chart.layout.labels.quadrantLabels.br.color},bl:{x:e.value.left+e.value.usableWidth/4-20,y:20,fill:l.value.style.chart.layout.labels.quadrantLabels.bl.color},crosshairs:{horizontal:`M ${e.value.left+e.value.usableWidth/4-20},20 ${e.value.left+e.value.usableWidth/4+20},20`,vertical:`M ${e.value.left+e.value.usableWidth/4},0 ${e.value.left+e.value.usableWidth/4},40`}},TR:{tl:{x:e.value.centerX+e.value.usableWidth/4-20,y:0,fill:l.value.style.chart.layout.labels.quadrantLabels.tl.color},tr:{x:e.value.centerX+e.value.usableWidth/4,y:0,fill:l.value.style.chart.layout.labels.quadrantLabels.tr.color},br:{x:e.value.centerX+e.value.usableWidth/4,y:20,fill:l.value.style.chart.layout.labels.quadrantLabels.br.color},bl:{x:e.value.centerX+e.value.usableWidth/4-20,y:20,fill:l.value.style.chart.layout.labels.quadrantLabels.bl.color},crosshairs:{horizontal:`M ${e.value.centerX+e.value.usableWidth/4-20},20 ${e.value.centerX+e.value.usableWidth/4+20},20`,vertical:`M ${e.value.centerX+e.value.usableWidth/4},0 ${e.value.centerX+e.value.usableWidth/4},40`}},BR:{tl:{x:e.value.centerX+e.value.usableWidth/4-20,y:e.value.centerY-48,fill:l.value.style.chart.layout.labels.quadrantLabels.tl.color},tr:{x:e.value.centerX+e.value.usableWidth/4,y:e.value.centerY-48,fill:l.value.style.chart.layout.labels.quadrantLabels.tr.color},br:{x:e.value.centerX+e.value.usableWidth/4,y:e.value.centerY-28,fill:l.value.style.chart.layout.labels.quadrantLabels.br.color},bl:{x:e.value.centerX+e.value.usableWidth/4-20,y:e.value.centerY-28,fill:l.value.style.chart.layout.labels.quadrantLabels.bl.color},crosshairs:{horizontal:`M ${e.value.centerX+e.value.usableWidth/4-20},${e.value.centerY-28} ${e.value.centerX+e.value.usableWidth/4+20},${e.value.centerY-28}`,vertical:`M ${e.value.centerX+e.value.usableWidth/4},${e.value.centerY-48} ${e.value.centerX+e.value.usableWidth/4},${e.value.centerY-8}`}},BL:{tl:{x:e.value.left+e.value.usableWidth/4-20,y:e.value.centerY-48,fill:l.value.style.chart.layout.labels.quadrantLabels.tl.color},tr:{x:e.value.left+e.value.usableWidth/4,y:e.value.centerY-48,fill:l.value.style.chart.layout.labels.quadrantLabels.tr.color},br:{x:e.value.left+e.value.usableWidth/4,y:e.value.centerY-28,fill:l.value.style.chart.layout.labels.quadrantLabels.br.color},bl:{x:e.value.left+e.value.usableWidth/4-20,y:e.value.centerY-28,fill:l.value.style.chart.layout.labels.quadrantLabels.bl.color},crosshairs:{horizontal:`M ${e.value.left+e.value.usableWidth/4-20},${e.value.centerY-28} ${e.value.left+e.value.usableWidth/4+20},${e.value.centerY-28}`,vertical:`M ${e.value.left+e.value.usableWidth/4},${e.value.centerY-48} ${e.value.left+e.value.usableWidth/4},${e.value.centerY-8}`}}}));function Ye(){return C.value.map(a=>({color:a.color,name:a.name,shape:a.shape,series:a.series.map(o=>({name:o.name,x:o.xValue,y:o.yValue,quadrantSide:o.quadrantSide,sideName:l.value.style.chart.layout.labels.quadrantLabels[o.quadrantSide].text}))}))}function Ae(){il(()=>{const a=[[l.value.style.chart.title.text],[l.value.style.chart.title.subtitle.text],[""]],o=ee.value.head,t=ee.value.body,s=a.concat([o]).concat(t),r=nl(s);vl({csvContent:r,title:l.value.style.chart.title.text||"vue-ui-quadrant"})})}const U=y(!1);function Ke(a){U.value=a,ce.value+=1}function _e(){w.value.showTable=!w.value.showTable}function Me(){w.value.plotLabels.show=!w.value.plotLabels.show}function Ne(){w.value.showTooltip=!w.value.showTooltip}const ae=y(!1);function ie(){ae.value=!ae.value}return Pe({getData:Ye,generatePdf:Le,generateCsv:Ae,generateImage:we,toggleTable:_e,toggleLabels:Me,toggleTooltip:Ne,toggleAnnotator:ie}),(a,o)=>(u(),i("div",{class:T(`vue-ui-quadrant ${U.value?"vue-data-ui-wrapper-fullscreen":""} ${l.value.useCssAnimation?"":"vue-ui-dna"}`),ref_key:"quadrantChart",ref:R,id:`vue-ui-quadrant_${W.value}`,style:z(`font-family:${l.value.style.fontFamily};width:100%; text-align:center;background:${l.value.style.chart.backgroundColor};${l.value.responsive?"height: 100%":""}`)},[l.value.userOptions.buttons.annotator?(u(),B(Tl,{key:0,parent:R.value,backgroundColor:l.value.style.chart.backgroundColor,color:l.value.style.chart.color,active:ae.value,onClose:ie},null,8,["parent","backgroundColor","color","active"])):d("",!0),Ge.value?(u(),i("div",{key:1,ref_key:"noTitle",ref:be,class:"vue-data-ui-no-title-space",style:"height:36px; width: 100%;background:transparent"},null,512)):d("",!0),l.value.style.chart.title.text?(u(),i("div",{key:2,ref_key:"chartTitle",ref:de,style:"width:100%;background:transparent;padding-bottom:12px"},[(u(),B($l,{key:`table_${pe.value}`,config:{title:{cy:"quadrant-title",...l.value.style.chart.title},subtitle:{cy:"quadrant-subtitle",...l.value.style.chart.title.subtitle}}},null,8,["config"]))],512)):d("",!0),l.value.userOptions.show&&Q.value?(u(),B(Ll,{ref_key:"details",ref:Ee,key:`user_options_${ce.value}`,backgroundColor:l.value.style.chart.backgroundColor,color:l.value.style.chart.color,isImaging:h(ke),isPrinting:h($e),uid:W.value,hasTooltip:l.value.userOptions.buttons.tooltip&&l.value.style.chart.tooltip.show,hasPdf:l.value.userOptions.buttons.pdf,hasImg:l.value.userOptions.buttons.img,hasXls:l.value.userOptions.buttons.csv,hasTable:l.value.userOptions.buttons.table,hasLabel:l.value.userOptions.buttons.labels,hasFullscreen:l.value.userOptions.buttons.fullscreen,isFullscreen:U.value,isTooltip:w.value.showTooltip,titles:{...l.value.userOptions.buttonTitles},chartElement:R.value,position:l.value.userOptions.position,hasAnnotator:l.value.userOptions.buttons.annotator,isAnnotation:ae.value,onToggleFullscreen:Ke,onGeneratePdf:h(Le),onGenerateCsv:Ae,onGenerateImage:h(we),onToggleTable:_e,onToggleLabels:Me,onToggleTooltip:Ne,onToggleAnnotator:ie},cl({_:2},[a.$slots.optionTooltip?{name:"optionTooltip",fn:x(()=>[L(a.$slots,"optionTooltip",{},void 0,!0)]),key:"0"}:void 0,a.$slots.optionPdf?{name:"optionPdf",fn:x(()=>[L(a.$slots,"optionPdf",{},void 0,!0)]),key:"1"}:void 0,a.$slots.optionCsv?{name:"optionCsv",fn:x(()=>[L(a.$slots,"optionCsv",{},void 0,!0)]),key:"2"}:void 0,a.$slots.optionImg?{name:"optionImg",fn:x(()=>[L(a.$slots,"optionImg",{},void 0,!0)]),key:"3"}:void 0,a.$slots.optionTable?{name:"optionTable",fn:x(()=>[L(a.$slots,"optionTable",{},void 0,!0)]),key:"4"}:void 0,a.$slots.optionLabels?{name:"optionLabels",fn:x(()=>[L(a.$slots,"optionLabels",{},void 0,!0)]),key:"5"}:void 0,a.$slots.optionFullscreen?{name:"optionFullscreen",fn:x(({toggleFullscreen:t,isFullscreen:s})=>[L(a.$slots,"optionFullscreen",D(J({toggleFullscreen:t,isFullscreen:s})),void 0,!0)]),key:"6"}:void 0,a.$slots.optionAnnotator?{name:"optionAnnotator",fn:x(({toggleAnnotator:t,isAnnotator:s})=>[L(a.$slots,"optionAnnotator",D(J({toggleAnnotator:t,isAnnotator:s})),void 0,!0)]),key:"7"}:void 0]),1032,["backgroundColor","color","isImaging","isPrinting","uid","hasTooltip","hasPdf","hasImg","hasXls","hasTable","hasLabel","hasFullscreen","isFullscreen","isTooltip","titles","chartElement","position","hasAnnotator","isAnnotation","onGeneratePdf","onGenerateImage"])):d("",!0),Q.value?(u(),i("svg",{key:4,xmlns:h(dl),class:T({"vue-data-ui-fullscreen--on":U.value,"vue-data-ui-fulscreen--off":!U.value}),viewBox:`${c.value.startX} ${c.value.startY} ${c.value.width} ${c.value.height}`,style:z(`max-width:100%;overflow:${g.value?"hidden":"visible"};background:transparent;color:${l.value.style.chart.color}`),id:`svg_${W.value}`},[ve(Wl),v("defs",null,[(u(!0),i(S,null,A(C.value,(t,s)=>(u(),i("radialGradient",{cx:"50%",cy:"50%",r:"50%",fx:"50%",fy:"50%",id:`quadrant_gradient_${W.value}_${s}`},[v("stop",{offset:"0%","stop-color":h(te)(h(yl)(t.color,.05),l.value.style.chart.layout.areas.opacity)},null,8,Ml),v("stop",{offset:"100%","stop-color":h(te)(t.color,l.value.style.chart.layout.areas.opacity)},null,8,Nl)],8,_l))),256))]),l.value.style.chart.layout.grid.graduations.show?(u(),i("g",Vl,[(u(!0),i(S,null,A(De.value,t=>(u(),i("rect",{fill:l.value.style.chart.layout.grid.graduations.fill?h(te)(l.value.style.chart.layout.grid.graduations.color,t.opacity):"none",x:t.x,y:t.y,height:t.height<=0?.001:t.height,width:t.width<=0?.001:t.width,"stroke-width":l.value.style.chart.layout.grid.graduations.strokeWidth,stroke:l.value.style.chart.layout.grid.graduations.stroke,rx:l.value.style.chart.layout.grid.graduations.roundingForce},null,8,Bl))),256))])):d("",!0),v("line",{x1:e.value.left,y1:e.value.centerY,x2:e.value.right,y2:e.value.centerY,stroke:l.value.style.chart.layout.grid.stroke,"stroke-width":l.value.style.chart.layout.grid.strokeWidth},null,8,Hl),v("line",{x1:e.value.centerX,y1:e.value.top,x2:e.value.centerX,y2:e.value.bottom,stroke:l.value.style.chart.layout.grid.stroke,"stroke-width":l.value.style.chart.layout.grid.strokeWidth},null,8,Rl),l.value.style.chart.layout.grid.showArrows?(u(),i("g",Ol,[v("polygon",{points:`${e.value.right-8},${e.value.centerY-6} ${e.value.right},${e.value.centerY} ${e.value.right-8},${e.value.centerY+6}`,fill:l.value.style.chart.layout.grid.stroke,stroke:"none"},null,8,Il),v("polygon",{points:`${e.value.left+8},${e.value.centerY-6} ${e.value.left},${e.value.centerY} ${e.value.left+8},${e.value.centerY+6}`,fill:l.value.style.chart.layout.grid.stroke,stroke:"none"},null,8,Pl),v("polygon",{points:`${e.value.centerX-6},${e.value.top+8} ${e.value.centerX},${e.value.top} ${e.value.centerX+6},${e.value.top+8}`,fill:l.value.style.chart.layout.grid.stroke,stroke:"none"},null,8,Fl),v("polygon",{points:`${e.value.centerX-6},${e.value.bottom-8} ${e.value.centerX},${e.value.bottom} ${e.value.centerX+6},${e.value.bottom-8}`,fill:l.value.style.chart.layout.grid.stroke,stroke:"none"},null,8,El)])):d("",!0),l.value.style.chart.layout.labels.quadrantLabels.show&&!g.value?(u(),i("g",Gl,[l.value.style.chart.layout.labels.quadrantLabels.tl.text?(u(),i("text",{key:0,x:0,y:e.value.top-e.value.padding/2,"text-anchor":"start",fill:l.value.style.chart.layout.labels.quadrantLabels.tl.color,"font-size":l.value.style.chart.layout.labels.quadrantLabels.tl.fontSize,style:z(`font-weight:${l.value.style.chart.layout.labels.quadrantLabels.tl.bold?"bold":""}`),onClick:o[0]||(o[0]=t=>G("tl"))},$(l.value.style.chart.layout.labels.quadrantLabels.tl.text),13,Ul)):d("",!0),l.value.style.chart.layout.labels.quadrantLabels.tr.text?(u(),i("text",{key:1,x:e.value.width,y:e.value.top-e.value.padding/2,"text-anchor":"end",fill:l.value.style.chart.layout.labels.quadrantLabels.tr.color,"font-size":l.value.style.chart.layout.labels.quadrantLabels.tr.fontSize,style:z(`font-weight:${l.value.style.chart.layout.labels.quadrantLabels.tr.bold?"bold":""}`),onClick:o[1]||(o[1]=t=>G("tr"))},$(l.value.style.chart.layout.labels.quadrantLabels.tr.text),13,Dl)):d("",!0),l.value.style.chart.layout.labels.quadrantLabels.br.text?(u(),i("text",{key:2,x:e.value.width,y:e.value.bottom+e.value.padding*.7,"text-anchor":"end",fill:l.value.style.chart.layout.labels.quadrantLabels.br.color,"font-size":l.value.style.chart.layout.labels.quadrantLabels.br.fontSize,style:z(`font-weight:${l.value.style.chart.layout.labels.quadrantLabels.br.bold?"bold":""}`),onClick:o[2]||(o[2]=t=>G("br"))},$(l.value.style.chart.layout.labels.quadrantLabels.br.text),13,Jl)):d("",!0),l.value.style.chart.layout.labels.quadrantLabels.bl.text?(u(),i("text",{key:3,x:0,y:e.value.bottom+e.value.padding*.7,"text-anchor":"start",fill:l.value.style.chart.layout.labels.quadrantLabels.bl.color,"font-size":l.value.style.chart.layout.labels.quadrantLabels.bl.fontSize,style:z(`font-weight:${l.value.style.chart.layout.labels.quadrantLabels.bl.bold?"bold":""}`),onClick:o[3]||(o[3]=t=>G("bl"))},$(l.value.style.chart.layout.labels.quadrantLabels.bl.text),13,Ql)):d("",!0)])):d("",!0),l.value.style.chart.layout.labels.axisLabels.show?(u(),i("g",jl,[v("text",{x:e.value.centerX,y:e.value.top-e.value.padding/6,"text-anchor":"middle","font-size":l.value.style.chart.layout.labels.axisLabels.fontSize,fill:l.value.style.chart.layout.labels.axisLabels.color.positive},$(h(N)(l.value.style.chart.layout.labels.plotLabels.y.formatter,q.value.y.max,h(V)({v:q.value.y.max,r:l.value.style.chart.layout.labels.plotLabels.rounding}))),9,Kl),v("text",{x:e.value.centerX,y:e.value.top-e.value.padding/2,"text-anchor":"middle","font-size":l.value.style.chart.layout.labels.axisLabels.fontSize,fill:l.value.style.chart.layout.labels.axisLabels.color.positive},$(l.value.style.chart.layout.grid.yAxis.name),9,Zl),v("text",{x:e.value.centerX,y:e.value.bottom+e.value.padding*.35,"text-anchor":"middle","font-size":l.value.style.chart.layout.labels.axisLabels.fontSize,fill:l.value.style.chart.layout.labels.axisLabels.color.negative},$(h(N)(l.value.style.chart.layout.labels.plotLabels.y.formatter,q.value.y.min,h(V)({v:q.value.y.min,r:l.value.style.chart.layout.labels.plotLabels.rounding}))),9,ea),v("text",{id:`xLabelMin_${W.value}`,"text-anchor":"middle","font-size":l.value.style.chart.layout.labels.axisLabels.fontSize,transform:`translate(${e.value.padding-l.value.style.chart.layout.labels.axisLabels.fontSize}, ${e.value.height/2}), rotate(-90)`,fill:l.value.style.chart.layout.labels.axisLabels.color.negative},$(h(N)(l.value.style.chart.layout.labels.plotLabels.x.formatter,q.value.x.min,h(V)({v:q.value.x.min,r:l.value.style.chart.layout.labels.plotLabels.rounding}))),9,la),v("text",{id:`xLabelMax_${W.value}`,"text-anchor":"middle","font-size":l.value.style.chart.layout.labels.axisLabels.fontSize,transform:`translate(${e.value.width-e.value.padding+l.value.style.chart.layout.labels.axisLabels.fontSize}, ${e.value.height/2}), rotate(90)`,fill:l.value.style.chart.layout.labels.axisLabels.color.positive},$(h(N)(l.value.style.chart.layout.labels.plotLabels.x.formatter,q.value.x.max,h(V)({v:q.value.x.max,r:l.value.style.chart.layout.labels.plotLabels.rounding}))),9,aa),v("text",{id:`xLabelMaxName_${W.value}`,"text-anchor":"middle","font-size":l.value.style.chart.layout.labels.axisLabels.fontSize,transform:`translate(${e.value.width-l.value.style.chart.layout.labels.axisLabels.fontSize}, ${e.value.height/2}), rotate(90)`,fill:l.value.style.chart.layout.labels.axisLabels.color.positive},$(l.value.style.chart.layout.grid.xAxis.name),9,ta)])):d("",!0),l.value.style.chart.layout.areas.show?(u(),i("g",oa,[(u(!0),i(S,null,A(C.value,(t,s)=>(u(),i("g",null,[t.series.length>2?(u(),i("polygon",{key:0,"data-cy-quadrant-area":"",fill:l.value.style.chart.layout.areas.useGradient?`url(#quadrant_gradient_${W.value}_${s})`:h(te)(t.color,l.value.style.chart.layout.areas.opacity),points:h(hl)(t)},null,8,ua)):d("",!0)]))),256))])):d("",!0),v("g",null,[v("rect",{onClick:o[4]||(o[4]=t=>_("TL")),x:e.value.left,y:e.value.top,width:e.value.usableWidth/2<=0?.001:e.value.usableWidth/2,height:e.value.usableHeight/2<=0?.001:e.value.usableHeight/2,fill:"transparent",class:T({"vue-data-ui-zoom-plus":!g.value,"vue-data-ui-zoom-minus":g.value})},null,10,sa),v("rect",{onClick:o[5]||(o[5]=t=>_("TR")),x:e.value.centerX,y:e.value.top,width:e.value.usableWidth/2<=0?.001:e.value.usableWidth/2,height:e.value.usableHeight/2<=0?.001:e.value.usableHeight/2,fill:"transparent",class:T({"vue-data-ui-zoom-plus":!g.value,"vue-data-ui-zoom-minus":g.value})},null,10,ra),v("rect",{onClick:o[6]||(o[6]=t=>_("BR")),x:e.value.centerX,y:e.value.centerY,width:e.value.usableWidth/2<=0?.001:e.value.usableWidth/2,height:e.value.usableHeight/2<=0?.001:e.value.usableHeight/2,fill:"transparent",class:T({"vue-data-ui-zoom-plus":!g.value,"vue-data-ui-zoom-minus":g.value})},null,10,ia),v("rect",{onClick:o[7]||(o[7]=t=>_("BL")),x:e.value.left,y:e.value.centerY,width:e.value.usableWidth/2<=0?.001:e.value.usableWidth/2,height:e.value.usableHeight/2<=0?.001:e.value.usableHeight/2,fill:"transparent",class:T({"vue-data-ui-zoom-plus":!g.value,"vue-data-ui-zoom-minus":g.value})},null,10,na)]),l.value.style.chart.layout.labels.plotLabels.showAsTag?(u(),i(S,{key:6},[w.value.plotLabels.show?(u(),i("g",da,[(u(!0),i(S,null,A(C.value,(t,s)=>(u(),i(S,null,[(u(!0),i(S,null,A(t.series,r=>(u(),i("foreignObject",{style:{overflow:"visible"},height:"10",width:"100",x:r.x-50,y:r.y-l.value.style.chart.layout.labels.plotLabels.fontSize,onMouseover:b=>Te(t,r,s),onMouseleave:o[9]||(o[9]=b=>{j.value=!1,F.value=null,E.value=null}),onClick:b=>Xe(t,r)},[v("div",{style:z(`color:${h(gl)(t.color)};margin: 0 auto; font-size:${l.value.style.chart.layout.labels.plotLabels.fontSize}px; text-align:center;background:${t.color}; padding: 2px 4px; border-radius: 12px; height: ${l.value.style.chart.layout.labels.plotLabels.fontSize*1.5}px`)},$(r.name),5)],40,ya))),256))],64))),256))])):d("",!0)],64)):(u(),i(S,{key:5},[(u(!0),i(S,null,A(C.value,(t,s)=>(u(),i("g",null,[(u(!0),i(S,null,A(t.series,r=>(u(),B(Re,{color:t.color,isSelected:F.value&&r.uid===F.value,plot:r,radius:l.value.style.chart.layout.plots.radius/(g.value?1.5:1),shape:t.shape,stroke:l.value.style.chart.layout.plots.outline?l.value.style.chart.layout.plots.outlineColor:"none",strokeWidth:l.value.style.chart.layout.plots.outlineWidth,onMouseover:b=>Te(t,r,s),onMouseleave:o[8]||(o[8]=b=>{j.value=!1,F.value=null,E.value=null}),onClick:b=>Xe(t,r)},null,8,["color","isSelected","plot","radius","shape","stroke","strokeWidth","onMouseover","onClick"]))),256))]))),256)),w.value.plotLabels.show?(u(),i("g",va,[(u(!0),i(S,null,A(C.value,t=>(u(),i("g",null,[(u(!0),i(S,null,A(t.series,s=>(u(),i("text",{x:s.x,y:s.y+l.value.style.chart.layout.labels.plotLabels.offsetY+l.value.style.chart.layout.plots.radius,"text-anchor":"middle","font-size":l.value.style.chart.layout.labels.plotLabels.fontSize/(g.value?1.5:1),fill:l.value.style.chart.layout.labels.plotLabels.color},$(s.name),9,ca))),256))]))),256))])):d("",!0)],64)),g.value?(u(),i("g",ha,[n.value==="TL"?(u(),i("polygon",{key:0,points:`${e.value.left-1},${e.value.centerY} ${e.value.centerX},${e.value.centerY} ${e.value.centerX},${e.value.top-1} ${e.value.right},${e.value.top-1} ${e.value.right},${e.value.bottom} ${e.value.left-1},${e.value.bottom} ${e.value.left-1},${e.value.centerY}`,fill:l.value.style.chart.backgroundColor,style:{opacity:"1"}},null,8,ba)):d("",!0),n.value==="TR"?(u(),i("polygon",{key:1,points:`${e.value.left},${e.value.top-1} ${e.value.centerX},${e.value.top-1} ${e.value.centerX},${e.value.centerY} ${e.value.right+1},${e.value.centerY} ${e.value.right+1},${e.value.bottom} ${e.value.left},${e.value.bottom} ${e.value.left},${e.value.top-1}`,fill:l.value.style.chart.backgroundColor,style:{opacity:"1"}},null,8,pa)):d("",!0),n.value==="BR"?(u(),i("polygon",{key:2,points:`${e.value.left},${e.value.top} ${e.value.right+1},${e.value.top} ${e.value.right+1},${e.value.centerY} ${e.value.centerX},${e.value.centerY} ${e.value.centerX},${e.value.bottom+1} ${e.value.left},${e.value.bottom+1} ${e.value.left},${e.value.top}`,fill:l.value.style.chart.backgroundColor,style:{opacity:"1"}},null,8,ga)):d("",!0),n.value==="BL"?(u(),i("polygon",{key:3,points:`${e.value.left-1},${e.value.top} ${e.value.right},${e.value.top} ${e.value.right},${e.value.bottom+1} ${e.value.centerX},${e.value.bottom+1} ${e.value.centerX},${e.value.centerY} ${e.value.left-1},${e.value.centerY} ${e.value.left-1},${e.value.top}`,fill:l.value.style.chart.backgroundColor,style:{opacity:"1"}},null,8,fa)):d("",!0)])):d("",!0),n.value&&!Z.value?(u(),i("g",ma,[v("text",{x:H.value.x,y:H.value.y-H.value.fontSize/1.5,"font-size":H.value.fontSize/1.5,fill:H.value.fill,"text-anchor":"middle","font-weight":H.value.bold?"bold":"normal"},$(H.value.text),9,xa)])):d("",!0),g.value&&n.value?(u(),i("g",$a,[v("rect",{x:k.value[n.value].tl.x,y:k.value[n.value].tl.y,height:"20",width:"20",fill:k.value[n.value].tl.fill,style:z(`cursor: pointer; opacity: ${n.value==="TL"?1:.2}`),onClick:o[10]||(o[10]=t=>_("TL")),class:T({"vue-ui-quadrant-mini-map-cell":!0,"vue-ui-quadrant-mini-map-cell-selectable":n.value!=="TL"})},null,14,ka),v("rect",{x:k.value[n.value].tr.x,y:k.value[n.value].tr.y,height:"20",width:"20",fill:k.value[n.value].tr.fill,style:z(`cursor: pointer; opacity: ${n.value==="TR"?1:.2}`),onClick:o[11]||(o[11]=t=>_("TR")),class:T({"vue-ui-quadrant-mini-map-cell":!0,"vue-ui-quadrant-mini-map-cell-selectable":n.value!=="TR"})},null,14,La),v("rect",{x:k.value[n.value].br.x,y:k.value[n.value].br.y,height:"20",width:"20",fill:k.value[n.value].br.fill,style:z(`cursor: pointer; opacity: ${n.value==="BR"?1:.2}`),onClick:o[12]||(o[12]=t=>_("BR")),class:T({"vue-ui-quadrant-mini-map-cell":!0,"vue-ui-quadrant-mini-map-cell-selectable":n.value!=="BR"})},null,14,wa),v("rect",{x:k.value[n.value].bl.x,y:k.value[n.value].bl.y,height:"20",width:"20",fill:k.value[n.value].bl.fill,style:z(`cursor: pointer; opacity: ${n.value==="BL"?1:.2}`),onClick:o[13]||(o[13]=t=>_("BL")),class:T({"vue-ui-quadrant-mini-map-cell":!0,"vue-ui-quadrant-mini-map-cell-selectable":n.value!=="BL"})},null,14,qa),v("path",{class:"vue-ui-quadrant-minimap-crosshairs",stroke:l.value.style.chart.backgroundColor,"stroke-width":1,d:k.value[n.value].crosshairs.horizontal},null,8,za),v("path",{class:"vue-ui-quadrant-minimap-crosshairs",stroke:l.value.style.chart.backgroundColor,"stroke-width":1,d:k.value[n.value].crosshairs.vertical},null,8,Sa)])):d("",!0),L(a.$slots,"svg",{svg:e.value},void 0,!0)],14,Al)):d("",!0),a.$slots.watermark?(u(),i("div",Ca,[L(a.$slots,"watermark",D(J({isPrinting:h($e)||h(ke)})),void 0,!0)])):d("",!0),Q.value?d("",!0):(u(),B(Sl,{key:6,config:{type:"quadrant",style:{backgroundColor:l.value.style.chart.backgroundColor,quadrant:{grid:{color:l.value.style.chart.layout.grid.stroke},plots:{color:l.value.style.chart.layout.grid.stroke,radius:1}}}}},null,8,["config"])),v("div",{ref_key:"chartLegend",ref:ye},[l.value.style.chart.legend.show?(u(),B(ql,{key:`legend_${fe.value}`,legendSet:Ce.value,config:je.value,onClickMarker:o[14]||(o[14]=({legend:t})=>se(t.id))},{item:x(({legend:t})=>[v("div",{"data-cy-legend-item":"",onClick:s=>se(t.id),style:z(`opacity:${M.value.includes(t.id)?.5:1}`)},$(t.name),13,Wa)]),_:1},8,["legendSet","config"])):L(a.$slots,"legend",{key:1,legend:Ce.value},void 0,!0)],512),a.$slots.source?(u(),i("div",{key:7,ref_key:"source",ref:he,dir:"auto"},[L(a.$slots,"source",{},void 0,!0)],512)):d("",!0),ve(wl,{show:w.value.showTooltip&&j.value,backgroundColor:l.value.style.chart.tooltip.backgroundColor,color:l.value.style.chart.tooltip.color,borderRadius:l.value.style.chart.tooltip.borderRadius,borderColor:l.value.style.chart.tooltip.borderColor,borderWidth:l.value.style.chart.tooltip.borderWidth,fontSize:l.value.style.chart.tooltip.fontSize,backgroundOpacity:l.value.style.chart.tooltip.backgroundOpacity,position:l.value.style.chart.tooltip.position,offsetY:l.value.style.chart.tooltip.offsetY,parent:R.value,content:ue.value,isCustom:l.value.style.chart.tooltip.customFormat&&typeof l.value.style.chart.tooltip.customFormat=="function"},{"tooltip-before":x(()=>[L(a.$slots,"tooltip-before",D(J({...re.value})),void 0,!0)]),"tooltip-after":x(()=>[L(a.$slots,"tooltip-after",D(J({...re.value})),void 0,!0)]),default:x(()=>[l.value.style.chart.tooltip.showShape?(u(),i("svg",Ta,[ve(Re,{plot:{x:10,y:10},shape:E.value.shape,color:E.value.color,radius:8,stroke:l.value.style.chart.layout.plots.outline?l.value.style.chart.layout.plots.outlineColor:"none","stroke-width":l.value.style.chart.layout.plots.outlineWidth},null,8,["shape","color","stroke","stroke-width"])])):d("",!0)]),_:3},8,["show","backgroundColor","color","borderRadius","borderColor","borderWidth","fontSize","backgroundOpacity","position","offsetY","parent","content","isCustom"]),Q.value?(u(),B(Cl,{key:8,hideDetails:"",config:{open:w.value.showTable,maxHeight:1e4,body:{backgroundColor:l.value.style.chart.backgroundColor,color:l.value.style.chart.color},head:{backgroundColor:l.value.style.chart.backgroundColor,color:l.value.style.chart.color}}},{content:x(()=>[(u(),B(zl,{key:`table_${ge.value}`,colNames:le.value.colNames,head:le.value.head,body:le.value.body,config:le.value.config,title:`${l.value.style.chart.title.text}${l.value.style.chart.title.subtitle.text?` : ${l.value.style.chart.title.subtitle.text}`:""}`,onClose:o[15]||(o[15]=t=>w.value.showTable=!1)},{th:x(({th:t})=>[fl($(t),1)]),td:x(({td:t})=>[v("div",{innerHTML:t.name||t},null,8,Xa)]),_:1},8,["colNames","head","body","config","title"]))]),_:1},8,["config"])):d("",!0)],14,Yl))}},Ua=Xl(Ya,[["__scopeId","data-v-557eff8e"]]);export{Ua as default};
