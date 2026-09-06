return (async () => {
  const keys = Object.keys(eda).filter(k => /sch|Primitive|Schematic/i.test(k)).sort();
  const schKeys = {};
  for (const k of keys) {
    if (eda[k] && typeof eda[k] === 'object') schKeys[k] = Object.keys(eda[k]).filter(m => typeof eda[k][m] === 'function');
  }
  return { ok: true, keyCount: keys.length, schKeys };
})();
