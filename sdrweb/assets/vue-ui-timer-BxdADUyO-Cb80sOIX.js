import{c as K}from"./useNestedProp-BxrcwmTZ-D4WcyZ8s.js";import{z as Q,r as c,D as Y,b as Z,c as H,ab as O,ac as ee,w as te,o as s,a as r,M as w,i as ae,L as h,u as le,J as se,f as M,e as $,v as q,m as A,p as J,q as U,h as oe}from"./index-Cv_R0XUS.js";import{O as re,k as ue}from"./useResponsive-CoxXLe23-CoJWR8uA.js";import{s as ie,u as P}from"./vue-data-ui-DF7u-eZV-BhlwTQFl.js";import{x as ne}from"./Title-DyD-mBPN-CJkaoNbn.js";import{p as ce}from"./PackageVersion-mSsg8dpu-Bg2p7G7d.js";class ve{constructor(B,f,C,D=!0,S=!0){this.interval=f,this.elapsed=0,this.isPaused=!1;const _=`
            let interval;
            let elapsed = 0;
            let paused = false;
            let startTime;
            let tickInterval;

            onmessage = function(e) {
                const { action, data } = e.data;

                switch(action) {
                    case 'start':
                        startTime = Date.now();
                        tickInterval = data.interval;
                        elapsed = 0;
                        paused = false;
                        interval = setInterval(() => {
                            elapsed += tickInterval;
                            postMessage({ elapsed, timestamp: Date.now() });
                        }, tickInterval);
                        break;
                    
                    case 'pause':
                        paused = true;
                        clearInterval(interval);
                        elapsed = Date.now() - startTime;
                        break;

                    case 'resume':
                        if (paused) {
                            startTime = Date.now() - elapsed;
                            interval = setInterval(() => {
                                elapsed += tickInterval;
                                postMessage({ elapsed, timestamp: Date.now() });
                            }, tickInterval);
                        }
                        paused = false;
                        break;

                    case 'stop':
                        clearInterval(interval);
                        elapsed = 0;
                        postMessage({ elapsed });
                        break;

                    case 'reset':
                        elapsed = 0;
                        clearInterval(interval);
                        postMessage({ elapsed });
                        break;

                    case 'lap':
                        postMessage({
                            elapsed,
                            timestamp: Date.now(),
                            action: 'lap'
                        });
                        break;

                    default:
                        break;
                }
            };
        `,T=new Blob([_],{type:"application/javascript"}),g=URL.createObjectURL(T),i=new Worker(g);function I(v){let e=Math.floor(v/1e3),k=Math.floor(v%1e3/10),y=Math.floor(e/3600),t=Math.floor(e%3600/60),p=e%60,u="";return S&&(u+=String(y).padStart(2,"0")+":"),u+=String(t).padStart(2,"0")+":",u+=String(p).padStart(2,"0"),D&&(u+="."+String(k).padStart(2,"0")),u}this.start=()=>{this.isPaused=!1,i.postMessage({action:"start",data:{interval:this.interval}})},this.pause=()=>{this.isPaused?this.resume():(this.isPaused=!0,i.postMessage({action:"pause"}))},this.resume=()=>{this.isPaused&&(i.postMessage({action:"resume"}),this.isPaused=!1)},this.stop=()=>{i.postMessage({action:"stop"}),this.isPaused=!1},this.reset=()=>{i.postMessage({action:"reset"}),this.elapsed=0,this.isPaused=!1},this.restart=()=>{this.stop(),this.start()},this.lap=()=>new Promise(v=>{i.postMessage({action:"lap"});const e=k=>{const{elapsed:y,timestamp:t,action:p}=k.data;if(p==="lap"){const u=I(y);v({timestamp:t||0,elapsed:y,formatted:u})}};i.addEventListener("message",e,{once:!0})}),i.onmessage=v=>{const{elapsed:e,timestamp:k}=v.data;this.elapsed=e,B({timestamp:k||0,elapsed:this.elapsed,formatted:I(this.elapsed)})},i.onerror=v=>{C&&C(v)}}}const pe=["xmlns","viewBox"],he={key:0},de=["id"],we=["stop-color"],ke=["stop-color"],fe=["cx","cy","r","fill","stroke","stroke-width"],me=["d","stroke","stroke-width"],ge=["r","fill","stroke","stroke-width"],ye=["r","fill","stroke","stroke-width"],be=["x","y"],xe=["x","y","font-size","fill","font-weight"],Me={key:0,class:"vue-ui-timer-controls"},$e=["title"],Ce=["title"],Se=["title"],Ie=["title"],Pe=["title"],_e={__name:"vue-ui-timer",props:{config:{type:Object,default(){return{}}}},emits:["start","pause","reset","restart","lap"],setup(V,{expose:B,emit:f}){const C=V,{vue_ui_timer:D}=Q(),S=c(null),_=c(null),T=c(null),g=c(null),i=c(Y()),I=c(0);Z(()=>{v()});function v(){if(e.value.responsive){const a=re(()=>{const{width:o,height:n}=ue({chart:S.value,title:e.value.style.title.text?_.value:null,legend:T.value});t.value.width=o,t.value.height=n,t.value.tracker.core=O({relator:Math.min(o,n),adjuster:e.value.style.width,source:6*e.value.stopwatch.tracker.radiusRatio,threshold:1,fallback:1}),t.value.tracker.aura=O({relator:Math.min(o,n),adjuster:e.value.style.width,source:12*e.value.stopwatch.tracker.aura.radiusRatio,threshold:1,fallback:1}),t.value.label=O({relator:Math.min(o,n),adjuster:e.value.style.width,source:e.value.stopwatch.label.fontSize,threshold:10,fallback:10})});g.value=new ResizeObserver(a),g.value.observe(S.value.parentNode)}}ee(()=>{g.value&&g.value.disconnect()});const e=H({get:()=>k(),set:a=>a});function k(){return K({userConfig:C.config,defaultConfig:D})}te(()=>C.config,a=>{e.value=k(),v(),I.value+=1},{deep:!0});const y=H(()=>{if(e.value.stopwatch.showHours&&e.value.stopwatch.showHundredth)return"00:00:00.00";if(e.value.stopwatch.showHours&&!e.value.stopwatch.showHundredth)return"00:00:00";if(!e.value.stopwatch.showHours&&e.value.stopwatch.showHundredth)return"00:00.00";if(!e.value.stopwatch.showHours&&!e.value.stopwatch.showHundredth)return"00:00"}),t=c({height:e.value.style.height,width:e.value.style.width,tracker:{core:6*e.value.stopwatch.tracker.radiusRatio,aura:12*e.value.stopwatch.tracker.aura.radiusRatio},label:e.value.stopwatch.label.fontSize}),p=c(0),u=new ve(a=>E(a),10,"",e.value.stopwatch.showHundredth,e.value.stopwatch.showHours),j=c(!0),l=c(!1),d=c(!1);function z(){f("start"),j.value&&u.start(),j.value=!1,l.value=!0}function F(){l.value&&(f("reset"),u.stop(),b.value=[],j.value=!0,l.value=!1)}function L(){d.value=!d.value,f("pause",p.value),u.pause()}function W(){l.value&&(d.value=!1,f("restart"),b.value=[],u.restart())}const b=c([]);async function R(){if(!l.value||d.value)return;const a=await u.lap();a&&(b.value.push(a),f("lap",b.value))}function E({timestamp:a,elapsed:o,formatted:n}){p.value={timestamp:a,elapsed:o,formatted:n}}const m=H(()=>Math.min(t.value.width,t.value.height)/2.5*e.value.stopwatch.track.radiusRatio);function G(a,o){const n=360/(o*1e3);return a*n%360}function X(a){let o=a*(Math.PI/180),n=t.value.width/2+m.value*Math.cos(o),N=t.value.height/2+m.value*Math.sin(o);return{cx:n,cy:N}}const x=H(()=>{const a=G(p.value.elapsed,e.value.stopwatch.cycleSeconds),{cx:o,cy:n}=X(a-90),N=a>180?1:0;return{cx:o||t.value.width/2,cy:n||t.value.height/2-m.value,largeArcFlag:N,sweepFlag:1}});return B({start:z,pause:L,reset:F,restart:W,lap:R}),(a,o)=>(s(),r("div",{ref_key:"timerChart",ref:S,class:"vue-ui-timer",style:w({fontFamily:e.value.style.fontFamily,width:"100%",height:e.value.responsive?"100%":"auto",textAlign:"center"})},[e.value.style.title.text?(s(),r("div",{key:0,ref_key:"chartTitle",ref:_,style:w({width:"100%",background:e.value.style.backgroundColor})},[(s(),ae(ne,{key:`title_${I.value}`,config:{title:{cy:"title",...e.value.style.title},subtitle:{cy:"subtitle",...e.value.style.title.subtitle}}},null,8,["config"]))],4)):h("",!0),(s(),r("svg",{xmlns:le(se),viewBox:`0 0 ${t.value.width<=0?10:t.value.width} ${t.value.height<=0?10:t.value.height}`,style:w({maxWidth:"100%",overflow:"visible",background:e.value.style.backgroundColor})},[M(ce),e.value.stopwatch.tracker.gradient.show?(s(),r("defs",he,[$("radialGradient",{id:`tracker_gradient_${i.value}`,cx:"50%",cy:"50%",r:"50%",fx:"50%",fy:"50%"},[$("stop",{offset:"0%","stop-color":e.value.stopwatch.tracker.gradient.color},null,8,we),$("stop",{offset:"100%","stop-color":e.value.stopwatch.tracker.fill},null,8,ke)],8,de)])):h("",!0),$("circle",{cx:t.value.width/2,cy:t.value.height/2,r:m.value,fill:e.value.stopwatch.track.fill,stroke:e.value.stopwatch.track.stroke,"stroke-width":e.value.stopwatch.track.strokeWidth},null,8,fe),e.value.stopwatch.cycleTrack.show?(s(),r("path",{key:1,d:`M ${t.value.width/2},${t.value.height/2-m.value} A ${m.value},${m.value} 0 ${x.value.largeArcFlag},${x.value.sweepFlag} ${x.value.cx},${x.value.cy}`,stroke:e.value.stopwatch.cycleTrack.stroke,"stroke-width":e.value.stopwatch.cycleTrack.strokeWidth,"stroke-linecap":"round",fill:"none"},null,8,me)):h("",!0),$("circle",q(x.value,{r:t.value.tracker.core,fill:e.value.stopwatch.tracker.gradient.show?`url(#tracker_gradient_${i.value})`:e.value.stopwatch.tracker.fill,stroke:e.value.stopwatch.tracker.stroke,"stroke-width":e.value.stopwatch.tracker.strokeWidth}),null,16,ge),e.value.stopwatch.tracker.aura.show?(s(),r("circle",q({key:2},x.value,{r:t.value.tracker.aura,fill:`${e.value.stopwatch.tracker.aura.fill}20`,stroke:e.value.stopwatch.tracker.aura.stroke,"stroke-width":e.value.stopwatch.tracker.aura.strokeWidth}),null,16,ye)):h("",!0),a.$slots.time?(s(),r("foreignObject",{key:3,x:t.value.width/2,y:t.value.height/2,height:"0.1",width:"0.1",style:{overflow:"visible"}},[A(a.$slots,"time",J(U({...p.value})),void 0,!0)],8,be)):(s(),r("text",{key:4,x:t.value.width/2,y:t.value.height/2+t.value.label/4,"font-size":t.value.label,"text-anchor":"middle",fill:e.value.stopwatch.label.color,"font-weight":e.value.stopwatch.label.bold?"bold":"normal",style:{"font-variant-numeric":"tabular-nums !important"}},oe(p.value.formatted||y.value),9,xe))],12,pe)),$("div",{ref_key:"chartLegend",ref:T,style:w({width:"100%",backgroundColor:e.value.stopwatch.legend.backgroundColor})},[a.$slots.controls?h("",!0):(s(),r("div",Me,[e.value.stopwatch.legend.buttons.start?(s(),r("button",{key:0,title:e.value.stopwatch.legend.buttonTitles.start,onClick:z,class:"vue-ui-timer-button",style:w({opacity:l.value?.2:1,cursor:l.value?"default":"pointer"})},[M(P,{name:"play",stroke:e.value.stopwatch.legend.buttons.iconColor},null,8,["stroke"])],12,$e)):h("",!0),e.value.stopwatch.legend.buttons.pause?(s(),r("button",{key:1,title:d.value?e.value.stopwatch.legend.buttonTitles.resume:e.value.stopwatch.legend.buttonTitles.pause,onClick:L,class:"vue-ui-timer-button",style:w({opacity:l.value?1:.2,cursor:l.value?"pointer":"default"})},[M(P,{name:"pause",stroke:e.value.stopwatch.legend.buttons.iconColor},null,8,["stroke"])],12,Ce)):h("",!0),e.value.stopwatch.legend.buttons.reset?(s(),r("button",{key:2,title:e.value.stopwatch.legend.buttonTitles.reset,onClick:F,class:"vue-ui-timer-button",style:w({opacity:l.value?1:.2,cursor:l.value?"pointer":"default"})},[M(P,{name:"stop",stroke:e.value.stopwatch.legend.buttons.iconColor},null,8,["stroke"])],12,Se)):h("",!0),e.value.stopwatch.legend.buttons.restart?(s(),r("button",{key:3,title:e.value.stopwatch.legend.buttonTitles.restart,onClick:W,class:"vue-ui-timer-button",style:w({opacity:l.value?1:.2,cursor:l.value?"pointer":"default"})},[M(P,{name:"restart",stroke:e.value.stopwatch.legend.buttons.iconColor},null,8,["stroke"])],12,Ie)):h("",!0),e.value.stopwatch.legend.buttons.lap?(s(),r("button",{key:4,title:e.value.stopwatch.legend.buttonTitles.lap,onClick:R,class:"vue-ui-timer-button",style:w({opacity:l.value&&!d.value?1:.2,cursor:l.value&&!d.value?"pointer":"default"})},[M(P,{name:"lap",stroke:e.value.stopwatch.legend.buttons.iconColor},null,8,["stroke"])],12,Pe)):h("",!0)])),A(a.$slots,"controls",J(U({start:z,pause:L,reset:F,restart:W,lap:R,laps:b.value,isRunning:l.value,isPaused:d.value,...p.value})),void 0,!0),A(a.$slots,"laps",J(U({laps:b.value,lap:R,isRunning:l.value,isPaused:d.value,...p.value})),void 0,!0)],4)],4))}},ze=ie(_e,[["__scopeId","data-v-8609e7e7"]]);export{ze as default};
