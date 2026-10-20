'use strict';
// Regression tests: client-only generation metadata and procedural geometry.
// No browser or ESP32 is needed to run these checks.
const {test}=require('node:test');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const vm=require('node:vm');
const path=require('node:path');

const html=fs.readFileSync(path.join(__dirname,'../public/index.html'),'utf8');
const scripts=[...html.matchAll(/<script[^>]*>([\s\S]*?)<\/script>/g)].map(m=>m[1]);
assert.equal(scripts.length,1);
const script=scripts[0];
function snippet(start,end){
  const a=script.indexOf(start),b=script.indexOf(end,a+1);
  assert.ok(a>=0,'Missing code section '+start);
  assert.ok(b>a,'Missing code section '+end);
  return script.slice(a,b);
}
const catalogs=snippet('const vehicleCatalog=','  const genericGeometry=');
const geometry=snippet('const genericGeometry=','  const vehiclePref=');
const toSvg=snippet('  function currentGeometry(){','  function setVehicleSelection(){');
const to3D=snippet('  function carQuad(', '  function projectCar(');
const catalog=new Function(catalogs+';return vehicleCatalog;')();
const make=new Function('metadata',
  "const paintData={silver:'#a8b5c2',white:'#e5eaf0',graphite:'#516373',blue:'#467da8'};let paint='silver';let vehicleMeta=metadata;"+
  geometry+toSvg+to3D+
  'return {svg:modelTopSvg(),mesh:modelMesh(),style:currentStyle()};'
);

test('ParkSense frontend code parses and has no duplicate static IDs',()=>{
  assert.doesNotThrow(()=>new vm.Script(script));
  const ids=[...html.matchAll(/\bid="([^"]+)"/g)].map(x=>x[1]);
  assert.equal(new Set(ids).size,ids.length);
});

test('Malaysia vehicle catalog has generation hierarchy for Myvi and Saga',()=>{
  assert.ok(Object.keys(catalog).length>=20);
  assert.ok(Object.keys(catalog.Perodua).length>=8);
  assert.ok(Object.keys(catalog.Proton).length>=8);
  assert.equal(catalog.Perodua.Myvi[1].length,6);
  assert.equal(catalog.Proton.Saga[1].length,4);
});

test('Every selectable model draws finite 2D SVG and 3D mesh',()=>{
  let count=0;
  for(const [brand,models] of Object.entries(catalog)){
    for(const [model,[body,generations]] of Object.entries(models)){
      for(const [generation,profile] of generations){
        const {svg,mesh}=make({brand,model,generation,body,profile});
        assert.ok(svg.startsWith('<svg '),brand+' '+model+' '+generation);
        assert.ok(svg.endsWith('</svg>'));
        assert.ok(mesh.length>60);
        for(const face of mesh){
          assert.match(face.paint,/^#[a-f\d]{6}$/i);
          assert.ok(face.points.length>=3);
          assert.ok(face.points.every(p=>p.length===3&&p.every(Number.isFinite)));
        }
        count++;
      }
    }
  }
  assert.ok(count>=200,'Missing Malaysian generation options');
});

test('Myvi and Saga major generations have different model geometry',()=>{
  for(const [brand,model] of [['Perodua','Myvi'],['Proton','Saga']]){
    const [body,gens]=catalog[brand][model],signatures=new Set();
    for(const [generation,profile] of gens){
      assert.ok(profile,'Every major '+model+' entry should have an original concept profile');
      const a=make({brand,model,generation,body,profile});
      signatures.add(JSON.stringify(a.mesh));
      assert.notEqual(a.style.line,'Generic hatchback visual');
      assert.notEqual(a.style.line,'Generic sedan visual');
    }
    assert.equal(signatures.size,gens.length,model+' generation silhouettes should differ');
  }
});
