#pragma once
#include <Arduino.h>

static const char PAGINA_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="es"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Monitor de Ruido</title>
<style>
:root{color-scheme:dark;font-family:system-ui,-apple-system,Segoe UI,sans-serif}
*{box-sizing:border-box}body{background:#0c1525;color:#e9eefb;max-width:970px;margin:0 auto;padding:22px}
h1{font-size:1.65rem;margin:0 0 4px}p{color:#aebdd5}h2{font-size:1.1rem;margin:0 0 16px}
header{margin-bottom:22px}.muted{font-size:.88rem;color:#9cabc5}.grid{display:grid;grid-template-columns:repeat(3,1fr);gap:12px}
.card{background:#17243a;border:1px solid #30425e;border-radius:13px;padding:17px;margin-bottom:14px}
.metric{font-size:2rem;font-weight:750;margin:4px 0}.flag{font-size:1.15rem;font-weight:650}
.two{display:grid;grid-template-columns:repeat(2,1fr);gap:12px}label{font-size:.87rem;color:#c1cde0;display:block;margin:9px 0}
input,select,button{display:block;width:100%;padding:10px;border:1px solid #405574;border-radius:8px;background:#0e1a2b;color:#fff;font-size:1rem}
input[type=checkbox]{display:inline;width:auto;margin-right:8px}button{background:#2878e6;border:none;cursor:pointer;font-weight:650}
button.secondary{background:#29405d}button:hover{filter:brightness(1.15)}.buttons{display:flex;gap:10px;flex-wrap:wrap}
.buttons button{width:auto}canvas{width:100%;height:180px;display:block}#mensaje{min-height:20px;font-size:.94rem}
@media(max-width:620px){.grid{grid-template-columns:1fr}.two{grid-template-columns:1fr}body{padding:12px}}
</style></head><body>
<header><h1>Monitor de ruido</h1><p>NodeMCU ESP8266 · control del ruido del pasillo</p></header>
<div class="grid">
  <section class="card"><div class="muted">Nivel estimado</div><div class="metric" id="nivel">—</div><div class="muted">Sin ponderación A; requiere calibración</div></section>
  <section class="card"><div class="muted">Estado</div><div class="flag" id="estado">—</div><div class="muted">Umbral alto: <span id="umbral">—</span> dB</div></section>
  <section class="card"><div class="muted">Puerta</div><div class="flag" id="puerta">—</div><div class="muted">Modo: <span id="modoActual">—</span></div></section>
</div>
<section class="card"><h2>Últimas mediciones</h2><canvas id="grafico"></canvas><p class="muted">Historial temporal solo en este navegador (máximo 60 muestras).</p></section>
<section class="card"><h2>Conexión</h2><div id="conexion">Consultando...</div><div class="muted" id="hora"></div></section>
<section class="card"><h2>Configuración y control</h2>
<form id="config">
<div class="two">
<label>Umbral alto (dB)<input name="umbral" type="number" min="10" max="100" step="0.1" required></label>
<label>Histéresis (dB)<input name="histeresis" type="number" min="1" max="15" step="0.1" required></label>
<label>Offset de calibración (dB)<input name="offset" type="number" min="-30" max="70" step="0.1" required></label>
<label>Tiempo de cierre (segundos)<input name="cierre" type="number" min="5" max="3600" required></label>
<label>Reapertura (segundos)<input name="reapertura" type="number" min="2" max="3600" required></label>
<label>Tolerancia a pausas (segundos)<input name="tolerancia" type="number" min="1" max="60" required></label>
<label>Preaviso buzzer (segundos)<input name="preaviso" type="number" min="1" max="30" required></label>
<label>Ruido para alerta Telegram (segundos)<input name="aviso" type="number" min="5" max="1800" required></label>
<label>Espera entre alertas (segundos)<input name="enfriamiento" type="number" min="60" max="3600" required></label>
<label>Inicio horario habitual (hora)<input name="horaInicio" type="number" min="0" max="23" required></label>
<label>Fin horario habitual (hora)<input name="horaFin" type="number" min="1" max="24" required></label>
<label>Modo de la puerta<select name="modo"><option value="AUTO">Automático</option><option value="ABIERTA">Forzar abierta</option><option value="CERRADA">Forzar cerrada</option></select></label>
</div>
<label><input name="finSemana" type="checkbox">Considerar actividad habitual los fines de semana</label>
<label><input name="silencio" type="checkbox">Silenciar buzzer</label>
<div class="buttons"><button type="submit">Guardar cambios</button><button type="button" class="secondary" id="telegram">Probar Telegram</button></div>
</form>
<p class="muted">Los cambios son temporales: al reiniciar se recuperan los valores del código. Los controles de puerta indican una orden simulada hasta conectar el servo.</p>
<div id="mensaje" role="status"></div></section>
<script>
const $=id=>document.getElementById(id);
const f=$('config'); const graf=$('grafico'); const historial=[];let inicializado=false;
const campos=['umbral','histeresis','offset','cierre','reapertura','tolerancia','preaviso','aviso','enfriamiento','horaInicio','horaFin','modo'];
function dibujar(){
 const dpr=window.devicePixelRatio||1, w=graf.clientWidth, h=180;
 graf.width=Math.round(w*dpr);graf.height=Math.round(h*dpr);
 const c=graf.getContext('2d');c.setTransform(dpr,0,0,dpr,0,0);c.clearRect(0,0,w,h);
 c.strokeStyle='#32445f';c.lineWidth=1;
 for(let i=0;i<4;i++){let y=15+i*(h-30)/3;c.beginPath();c.moveTo(0,y);c.lineTo(w,y);c.stroke();}
 if(historial.length<2)return;
 const min=Math.min(...historial)-4,max=Math.max(...historial)+4;
 c.strokeStyle='#69b1ff';c.lineWidth=2;c.beginPath();
 historial.forEach((v,i)=>{const x=8+i*(w-16)/59;const y=h-15-(v-min)/(max-min)*(h-30);if(i===0)c.moveTo(x,y);else c.lineTo(x,y);});c.stroke();
}
function cargarFormulario(c){
 for(const key of campos)f.elements[key].value=c[key];
 f.elements.finSemana.checked=c.finSemana;
 f.elements.silencio.checked=c.silencio;
}
async function actualizar(){
 try{
  const r=await fetch('/api/estado',{cache:'no-store'});if(!r.ok)throw Error('HTTP '+r.status);
  const d=await r.json();$('nivel').textContent=d.nivelDb.toFixed(1)+' dB';
  $('estado').textContent=d.estado;$('puerta').textContent=d.puerta;
  $('modoActual').textContent=d.modo;$('umbral').textContent=d.config.umbral;
  $('conexion').textContent=(d.wifi?'Wi-Fi conectado':'Sin Internet (modo AP)')+' · IP: '+d.ip;
  $('hora').textContent=d.horaValida ? d.hora+' · '+(d.fueraHorario?'FUERA de horario':'Horario habitual') : 'Hora no sincronizada: no se envían alertas automáticas';
  historial.push(d.nivelDb);if(historial.length>60)historial.shift();dibujar();
  if(!inicializado){cargarFormulario(d.config);inicializado=true;}
 }catch(e){$('conexion').textContent='No se pudo consultar el dispositivo: '+e.message;}
}
f.addEventListener('submit',async e=>{
 e.preventDefault();$('mensaje').textContent='Guardando...';
 const datos=new URLSearchParams(new FormData(f));datos.set('finSemana',f.elements.finSemana.checked?'1':'0');datos.set('silencio',f.elements.silencio.checked?'1':'0');
 try{const r=await fetch('/api/config',{method:'POST',body:datos});const txt=await r.text();if(!r.ok)throw Error(txt);$('mensaje').textContent='Configuración aplicada. '+txt;}
 catch(e){$('mensaje').textContent='Error: '+e.message;}
});
$('telegram').addEventListener('click',async()=>{
 $('mensaje').textContent='Enviando mensaje de prueba...';
 try{const r=await fetch('/api/prueba-telegram',{method:'POST'});const txt=await r.text();if(!r.ok)throw Error(txt);$('mensaje').textContent=txt;}
 catch(e){$('mensaje').textContent='Error: '+e.message;}
});
actualizar();setInterval(actualizar,1000);window.addEventListener('resize',dibujar);
</script></body></html>)HTML";
