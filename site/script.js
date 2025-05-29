let plantas = [];
let favoritas = [];

async function carregarPlantas() {
  try {
    const response = await fetch('plantas.json');
    plantas = await response.json();
    console.log('Plantas carregadas:', plantas);
  } catch (err) {
    console.error('Erro ao carregar plantas:', err);
  }
}

function atualizarDropdown(lista) {
  const dropdown = document.getElementById('dropdown');
  dropdown.innerHTML = '';
  if (lista.length === 0) {
    dropdown.classList.add('hidden');
    return;
  }
  lista.forEach(planta => {
    const li = document.createElement('li');
    li.textContent = planta.nome_popular;
    li.addEventListener('click', () => {
      adicionarFavorita(planta);
      dropdown.classList.add('hidden');
      document.getElementById('searchInput').value = '';
    });
    dropdown.appendChild(li);
  });
  dropdown.classList.remove('hidden');
}

function filtrarPlantas(query) {
  const q = query.toLowerCase();
  return plantas.filter(planta => planta.nome_popular.toLowerCase().includes(q));
}

function adicionarFavorita(planta) {
  if (favoritas.some(fav => fav.nome_popular === planta.nome_popular)) {
    alert('Planta já adicionada!');
    return;
  }
  if (favoritas.length >= 2) {
    alert('Você pode adicionar no máximo 2 plantas favoritas.');
    return;
  }
  favoritas.push(planta);
  atualizarFavoritas();
}

function atualizarFavoritas() {
  const ul = document.getElementById('favoritas');
  ul.innerHTML = '';
  favoritas.forEach(planta => {
    const li = document.createElement('li');
    li.textContent = `${planta.nome_popular} - Solo: ${planta.umidade_solo}, UV: ${planta.uv_dia}`;
    ul.appendChild(li);
  });
}

document.addEventListener('DOMContentLoaded', () => {
  carregarPlantas();

  const searchInput = document.getElementById('searchInput');
  const dropdown = document.getElementById('dropdown');

  searchInput.addEventListener('input', () => {
    const query = searchInput.value.trim();
    if (query.length === 0) {
      dropdown.classList.add('hidden');
      return;
    }
    const filtradas = filtrarPlantas(query);
    atualizarDropdown(filtradas);
  });
});
