import{z as $e,c as p,r as v,D as Se,R as ze,A as oe,H as Me,w as re,B as se,C as A,Z as ue,Q as Ce,E as _e,ab as ie,b as Pe,ac as Ie,F as ne,o as r,a as i,i as N,L as y,l as z,h as M,j as Oe,m,p as j,q as B,u as c,M as ve,n as ce,J as Ae,f as Ne,e as f,O as V,P as W,k as E,aj as X,U as Xe,G as Ge}from"./index-DIL_pLmJ.js";import{O as Re,k as Ye}from"./useResponsive-CoxXLe23-CoJWR8uA.js";import{r as je,a as Be}from"./usePrinter-CLcxWv-4-CCDxi2Mm.js";import Ve from"./vue-ui-skeleton-Dgzcn3FL-BNvPX0g2.js";import{x as We}from"./Title-DyD-mBPN-CHqvPevn.js";import{c as de}from"./useNestedProp-BxrcwmTZ-1Bk3HyKT.js";import{p as Ee}from"./PackageVersion-mSsg8dpu-BxnQ4_xj.js";import{S as qe}from"./PenAndPaper-Dk41ZrJ0-D5uht0-m.js";import{s as De}from"./vue-data-ui-DF7u-eZV-hcmE5A1B.js";const Te=["id"],Ue={key:0},He=["xmlns","viewBox"],Ze=["id"],Je=["stop-color"],Le=["stop-color"],Qe=["stop-color"],Ke=["id"],et=["stdDeviation"],tt=["d","fill","stroke"],at=["d","filter"],lt=["x","y","text-anchor","font-size","font-weight","fill"],ot=["x","y","font-size","font-weight","fill"],rt={key:1},st=["x1","y1","x2","y2","stroke","stroke-width"],ut=["x1","y1","x2","y2","stroke","stroke-width"],it=["x1","y1","x2","y2","stroke-width","filter"],nt={key:2},vt=["d","fill","stroke","stroke-width"],ct=["cx","cy","fill","r","stroke-width","stroke"],dt=["x","y","font-size","fill"],ht={key:5,class:"vue-data-ui-watermark"},yt={__name:"vue-ui-gauge",props:{config:{type:Object,default(){return{}}},dataset:{type:Object,default(){return{}}}},setup(G,{expose:he}){const s=G,{vue_ui_gauge:ye}=$e(),C=p(()=>!!s.dataset&&Object.keys(s.dataset).length>0&&s.dataset.series&&s.dataset.series.length),k=v(Se()),fe=v(null),q=v(0),F=v(null),D=v(null),T=v(null),U=v(null),H=v(null),Z=v(0),e=p({get:()=>J(),set:a=>a});re(()=>s.config,a=>{e.value=J(),ae(),Z.value+=1},{deep:!0});function J(){const a=de({userConfig:s.config,defaultConfig:ye});return a.theme?{...de({userConfig:Me.vue_ui_gauge[a.theme]||s.config,defaultConfig:a}),customPalette:ze[a.theme]||oe}:a}const{isPrinting:L,isImaging:Q,generatePdf:K,generateImage:ee}=je({elementId:`vue-ui-gauge_${k.value}`,fileName:e.value.style.chart.title.text||"vue-ui-gauge"}),ge=p(()=>e.value.userOptions.show&&!e.value.style.chart.title.text),pe=p(()=>_e(e.value.customPalette)),x=p(()=>{if(!C.value||se(s.dataset.series||{}))return{value:0,series:[{from:0,to:0}]};const a=[];(s.dataset.series||[]).forEach(l=>{a.push(l.from||0),a.push(l.to||0)});const o=Math.max(...a);return{...s.dataset,series:(s.dataset.series||[]).map((l,u)=>({...l,color:Ce(l.color)||pe.value[u]||oe[u],value:((l.to||0)-(l.from||0))/o*100}))}}),g=v(512),t=v({height:358.4,width:g.value,top:0,bottom:358.4,centerX:179.2,centerY:g.value/2,labelFontSize:18,legendFontSize:e.value.style.chart.legend.fontSize,pointerRadius:e.value.style.chart.layout.pointer.circle.radius,trackSize:e.value.style.chart.layout.track.size,pointerSize:e.value.style.chart.layout.pointer.size,pointerStrokeWidth:e.value.style.chart.layout.pointer.strokeWidth,markerOffset:e.value.style.chart.layout.markers.offsetY+3}),_=v(0),$=v(0),d=v(e.value.style.chart.animation.use?0:s.dataset.value);re(()=>s.dataset.value,()=>{le(s.dataset.value)});const h=p(()=>{const a=t.value.width/2,o=n.value.base,l=Math.PI*((d.value+0-$.value)/(_.value-$.value))+Math.PI;return{x1:a,y1:o,x2:a+n.value.pointerSize*t.value.pointerSize*.9*Math.cos(l),y2:o+n.value.pointerSize*t.value.pointerSize*.9*Math.sin(l)}}),te=p(()=>{const a=t.value.width/2,o=n.value.base,l=Math.PI*((d.value+0-$.value)/(_.value-$.value))+Math.PI,u=a+n.value.pointerSize*t.value.pointerSize*.9*Math.cos(l),O=o+n.value.pointerSize*t.value.pointerSize*.9*Math.sin(l),b=t.value.pointerRadius,be=a+b*Math.cos(l+Math.PI/2),xe=o+b*Math.sin(l+Math.PI/2),we=a+b*Math.cos(l-Math.PI/2),Fe=o+b*Math.sin(l-Math.PI/2);return isNaN(u)?null:`M ${u},${O} ${be},${xe} ${we},${Fe} Z`}),R=p(()=>{for(let a=0;a<x.value.series.length;a+=1){const{color:o,from:l,to:u}=x.value.series[a];if(d.value>=l&&d.value<=u)return o}return"#2D353C"}),P=v(null);function ae(){if(se(s.dataset)?A({componentName:"VueUiGauge",type:"dataset"}):(ue({datasetObject:s.dataset,requiredAttributes:["value","series"]}).forEach(a=>{A({componentName:"VueUiGauge",type:"datasetAttribute",property:a})}),Object.hasOwn(s.dataset,"series")&&(s.dataset.series.length?s.dataset.series.forEach((a,o)=>{ue({datasetObject:a,requiredAttributes:["from","to"]}).forEach(l=>{A({componentName:"VueUiGauge",type:"datasetSerieAttribute",property:l,index:o})})}):A({componentName:"VueUiGauge",type:"datasetAttributeEmpty",property:"series"}))),le(s.dataset.value||0),e.value.responsive){const a=Re(()=>{const{width:o,height:l}=Ye({chart:F.value,title:e.value.style.chart.title.text?D.value:null,legend:T.value,source:U.value,noTitle:H.value});t.value.width=o,t.value.height=l,t.value.centerX=o/2,t.value.centerY=g.value/2/358.4*l,t.value.bottom=l,t.value.labelFontSize=18/g.value*Math.min(l,o)<10?10:18/g.value*Math.min(l,o),t.value.legendFontSize=e.value.style.chart.legend.fontSize/g.value*Math.min(l,o)<14?14:e.value.style.chart.legend.fontSize/g.value*Math.min(l,o),t.value.pointerRadius=e.value.style.chart.layout.pointer.circle.radius/g.value*Math.min(l,o),t.value.trackSize=e.value.style.chart.layout.track.size/g.value*Math.min(l,o),t.value.pointerStrokeWidth=ie({relator:Math.min(o,l),adjuster:g.value,source:e.value.style.chart.layout.pointer.strokeWidth,threshold:2,fallback:2}),t.value.markerOffset=ie({relator:Math.max(o,l),adjuster:g.value,source:e.value.style.chart.layout.markers.offsetY+3,threshold:2,fallback:2})});P.value=new ResizeObserver(a),P.value.observe(F.value.parentNode)}}Pe(()=>{ae()}),Ie(()=>{P.value&&P.value.disconnect()});function le(a){const o=[];(x.value.series||[]).forEach(b=>{o.push(b.from||0),o.push(b.to||0)}),_.value=Math.max(...o),$.value=Math.min(...o);let l=e.value.style.chart.animation.speed;const u=Math.abs(a-d.value)/(l*60);function O(){d.value<a?d.value=Math.min(d.value+u,a):d.value>a&&(d.value=Math.max(d.value-u,a)),d.value!==a&&requestAnimationFrame(O)}O()}const n=p(()=>{const a=e.value.responsive?Math.min(t.value.width,t.value.height):t.value.width;return{arcs:a/2.5,gradients:a/2.75,base:e.value.responsive?t.value.height/2:t.value.height*.7,ratingBase:e.value.responsive?t.value.height/2+t.value.height/4:t.value.height*.9,pointerSize:e.value.responsive?Math.min(t.value.width,t.value.height)/3:t.value.width/3.2}}),w=p(()=>ne({series:x.value.series},t.value.width/2,n.value.base,n.value.arcs,n.value.arcs,1,1,1,180,109.9495,40*t.value.trackSize)),me=p(()=>ne({series:x.value.series},t.value.width/2,n.value.base,n.value.gradients,n.value.gradients,.95,1,1,180,110.02,2*t.value.trackSize)),S=v(!1);function ke(a){S.value=a,q.value+=1}const I=v(!1);function Y(){I.value=!I.value}return he({generatePdf:K,generateImage:ee,toggleAnnotator:Y}),(a,o)=>(r(),i("div",{class:ce(`vue-ui-gauge ${S.value?"vue-data-ui-wrapper-fullscreen":""}`),ref_key:"gaugeChart",ref:F,id:`vue-ui-gauge_${k.value}`,style:ve(`font-family:${e.value.style.fontFamily};width:100%; text-align:center;background:${e.value.style.chart.backgroundColor};${e.value.responsive?"height: 100%":""}`)},[e.value.userOptions.buttons.annotator?(r(),N(qe,{key:0,parent:F.value,backgroundColor:e.value.style.chart.backgroundColor,color:e.value.style.chart.color,active:I.value,onClose:Y},null,8,["parent","backgroundColor","color","active"])):y("",!0),ge.value?(r(),i("div",{key:1,ref_key:"noTitle",ref:H,class:"vue-data-ui-no-title-space",style:"height:36px; width: 100%;background:transparent"},null,512)):y("",!0),e.value.style.chart.title.text?(r(),i("div",{key:2,ref_key:"chartTitle",ref:D,style:"width:100%;background:transparent;padding-bottom:12px"},[(r(),N(We,{key:`title_${Z.value}`,config:{title:{cy:"gauge-div-title",...e.value.style.chart.title},subtitle:{cy:"gauge-div-subtitle",...e.value.style.chart.title.subtitle}}},{default:z(()=>[e.value.translations.base&&G.dataset.base?(r(),i("span",Ue,M(e.value.translations.base)+": "+M(G.dataset.base),1)):y("",!0)]),_:1},8,["config"]))],512)):y("",!0),e.value.userOptions.show&&C.value?(r(),N(Be,{ref_key:"details",ref:fe,key:`user_options_${q.value}`,backgroundColor:e.value.style.chart.backgroundColor,color:e.value.style.chart.color,isImaging:c(Q),isPrinting:c(L),uid:k.value,hasXls:!1,hasPdf:e.value.userOptions.buttons.pdf,hasImg:e.value.userOptions.buttons.img,hasFullscreen:e.value.userOptions.buttons.fullscreen,isFullscreen:S.value,titles:{...e.value.userOptions.buttonTitles},chartElement:F.value,position:e.value.userOptions.position,hasAnnotator:e.value.userOptions.buttons.annotator,isAnnotation:I.value,onToggleFullscreen:ke,onGeneratePdf:c(K),onGenerateImage:c(ee),onToggleAnnotator:Y},Oe({_:2},[a.$slots.optionPdf?{name:"optionPdf",fn:z(()=>[m(a.$slots,"optionPdf",{},void 0,!0)]),key:"0"}:void 0,a.$slots.optionImg?{name:"optionImg",fn:z(()=>[m(a.$slots,"optionImg",{},void 0,!0)]),key:"1"}:void 0,a.$slots.optionFullscreen?{name:"optionFullscreen",fn:z(({toggleFullscreen:l,isFullscreen:u})=>[m(a.$slots,"optionFullscreen",j(B({toggleFullscreen:l,isFullscreen:u})),void 0,!0)]),key:"2"}:void 0,a.$slots.optionAnnotator?{name:"optionAnnotator",fn:z(({toggleAnnotator:l,isAnnotator:u})=>[m(a.$slots,"optionAnnotator",j(B({toggleAnnotator:l,isAnnotator:u})),void 0,!0)]),key:"3"}:void 0]),1032,["backgroundColor","color","isImaging","isPrinting","uid","hasPdf","hasImg","hasFullscreen","isFullscreen","titles","chartElement","position","hasAnnotator","isAnnotation","onGeneratePdf","onGenerateImage"])):y("",!0),C.value?(r(),i("svg",{key:4,xmlns:c(Ae),class:ce({"vue-data-ui-fullscreen--on":S.value,"vue-data-ui-fulscreen--off":!S.value}),viewBox:`0 0 ${t.value.width<=0?10:t.value.width} ${t.value.height<=0?10:t.value.height}`,style:ve(`max-width:100%;overflow:hidden !important;background:transparent;color:${e.value.style.chart.color}`)},[Ne(Ee),f("defs",null,[f("radialGradient",{id:`gradient_${k.value}`,cx:"50%",cy:"50%",r:"50%",fx:"50%",fy:"50%"},[f("stop",{offset:"0%","stop-color":c(V)("#FFFFFF",1)},null,8,Je),f("stop",{offset:"80%","stop-color":c(V)("#FFFFFF",e.value.style.chart.layout.track.gradientIntensity)},null,8,Le),f("stop",{offset:"100%","stop-color":c(V)("#FFFFFF",1)},null,8,Qe)],8,Ze)]),f("defs",null,[f("filter",{id:`blur_${k.value}`,x:"-50%",y:"-50%",width:"200%",height:"200%"},[f("feGaussianBlur",{in:"SourceGraphic",stdDeviation:100/e.value.style.chart.layout.track.gradientIntensity},null,8,et)],8,Ke)]),(r(!0),i(W,null,E(w.value,(l,u)=>(r(),i("path",{key:`arc_${u}`,d:l.arcSlice,fill:l.color,stroke:e.value.style.chart.backgroundColor,"stroke-linecap":"round"},null,8,tt))),128)),e.value.style.chart.layout.track.useGradient?(r(!0),i(W,{key:0},E(me.value,(l,u)=>(r(),i("path",{key:`arc_${u}`,d:l.arcSlice,fill:"#FFFFFF",stroke:"none","stroke-linecap":"round",filter:`url(#blur_${k.value})`},null,8,at))),128)):y("",!0),(r(!0),i(W,null,E(w.value,(l,u)=>(r(),i("text",{x:c(X)({centerX:t.value.width/2,centerY:n.value.base,initX:l.center.startX,initY:l.center.startY,offset:t.value.markerOffset}).x,y:c(X)({centerX:t.value.width/2,centerY:n.value.base,initX:l.center.startX,initY:l.center.startY,offset:t.value.markerOffset}).y,"text-anchor":l.center.startX<t.value.width/2-5?"end":l.center.startX>t.value.width/2+5?"start":"middle","font-size":t.value.labelFontSize*e.value.style.chart.layout.markers.fontSizeRatio,"font-weight":`${e.value.style.chart.layout.markers.bold?"bold":"normal"}`,fill:e.value.style.chart.layout.markers.color},M(l.from.toFixed(e.value.style.chart.layout.markers.roundingValue)),9,lt))),256)),f("text",{x:c(X)({centerX:t.value.width/2,centerY:n.value.base,initX:w.value.at(-1).endX,initY:w.value.at(-1).endY,offset:t.value.markerOffset}).x,y:c(X)({centerX:t.value.width/2,centerY:n.value.base,initX:w.value.at(-1).endX,initY:w.value.at(-1).endY,offset:t.value.markerOffset}).y,"text-anchor":"start","font-size":t.value.labelFontSize*e.value.style.chart.layout.markers.fontSizeRatio,"font-weight":`${e.value.style.chart.layout.markers.bold?"bold":"normal"}`,fill:e.value.style.chart.layout.markers.color},M(_.value.toFixed(e.value.style.chart.layout.markers.roundingValue)),9,ot),e.value.style.chart.layout.pointer.type==="rounded"?(r(),i("g",rt,[isNaN(h.value.x2)?y("",!0):(r(),i("line",{key:0,x1:h.value.x1,y1:h.value.y1,x2:h.value.x2,y2:h.value.y2,stroke:e.value.style.chart.layout.pointer.stroke,"stroke-width":t.value.pointerStrokeWidth,"stroke-linecap":"round"},null,8,st)),isNaN(h.value.x2)?y("",!0):(r(),i("line",{key:1,x1:h.value.x1,y1:h.value.y1,x2:h.value.x2,y2:h.value.y2,stroke:e.value.style.chart.layout.pointer.useRatingColor?R.value:e.value.style.chart.layout.pointer.color,"stroke-linecap":"round","stroke-width":t.value.pointerStrokeWidth*.7},null,8,ut)),!isNaN(h.value.x2)&&e.value.style.chart.layout.track.useGradient?(r(),i("line",{key:2,x1:h.value.x1,y1:h.value.y1,x2:h.value.x2,y2:h.value.y2,stroke:"white","stroke-linecap":"round","stroke-width":t.value.pointerStrokeWidth*.3,filter:`url(#blur_${k.value})`},null,8,it)):y("",!0)])):(r(),i("g",nt,[te.value?(r(),i("path",{key:0,d:te.value,fill:e.value.style.chart.layout.pointer.useRatingColor?R.value:e.value.style.chart.layout.pointer.color,stroke:e.value.style.chart.layout.pointer.stroke,"stroke-width":e.value.style.chart.layout.pointer.circle.strokeWidth,"stroke-linejoin":"round"},null,8,vt)):y("",!0)])),f("circle",{cx:t.value.width/2,cy:n.value.base,fill:e.value.style.chart.layout.pointer.circle.color,r:t.value.pointerRadius<=0?1e-4:t.value.pointerRadius,"stroke-width":e.value.style.chart.layout.pointer.circle.strokeWidth,stroke:e.value.style.chart.layout.pointer.circle.stroke},null,8,ct),f("text",{x:t.value.width/2,y:n.value.ratingBase,"text-anchor":"middle","font-size":t.value.legendFontSize,"font-weight":"bold",fill:e.value.style.chart.legend.useRatingColor?R.value:e.value.style.chart.legend.color},M(c(Xe)(e.value.style.chart.legend.formatter,d.value,c(Ge)({p:e.value.style.chart.legend.prefix+(e.value.style.chart.legend.showPlusSymbol&&d.value>0?"+":""),v:d.value,s:e.value.style.chart.legend.suffix,r:e.value.style.chart.legend.roundingValue}))),9,dt),m(a.$slots,"svg",{svg:t.value},void 0,!0)],14,He)):y("",!0),a.$slots.watermark?(r(),i("div",ht,[m(a.$slots,"watermark",j(B({isPrinting:c(L)||c(Q)})),void 0,!0)])):y("",!0),C.value?y("",!0):(r(),N(Ve,{key:6,config:{type:"gauge",style:{backgroundColor:e.value.style.chart.backgroundColor,gauge:{color:"#CCCCCC"}}}},null,8,["config"])),f("div",{ref_key:"chartLegend",ref:T},[m(a.$slots,"legend",{legend:x.value},void 0,!0)],512),a.$slots.source?(r(),i("div",{key:7,ref_key:"source",ref:U,dir:"auto"},[m(a.$slots,"source",{},void 0,!0)],512)):y("",!0)],14,Te))}},$t=De(yt,[["__scopeId","data-v-1082d899"]]);export{$t as default};
