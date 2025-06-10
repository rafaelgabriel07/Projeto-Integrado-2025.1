let plantas = [];
let favoritas = [];

async function carregarPlantas() {
  try {
    const response = await fetch('plantas.json');
    plantas = await response.json();
    console.log('Plantas carregadas:', plantas);
  } catch (err) {
    console.error('Erro ao carregar plantas:', err);
    mostrarMensagemStatus('Erro ao carregar plantas. Verifique o arquivo plantas.json.', 'error');
  }
}

// NOVA FUNÇÃO: Enviar dados da planta para o ESP32 via POST
async function enviarDadosParaESP32(planta) {
  try {
    const response = await fetch('/selectPlant', {
      method: 'POST', // Define o método da requisição como POST
      headers: {
        'Content-Type': 'application/json' // Informa ao servidor que estamos enviando JSON
      },
      body: JSON.stringify(planta) // Converte o objeto JavaScript 'planta' em uma string JSON
    });

    if (response.ok) { // Verifica se a requisição foi bem-sucedida (status 2xx)
      const result = await response.text();
      console.log('Resposta do ESP32:', result);
      mostrarMensagemStatus('Dados da planta enviados com sucesso para o ESP32!', 'success');
    } else {
      console.error('Erro ao enviar dados para o ESP32:', response.status, response.statusText);
      const errorText = await response.text();
      console.error('Mensagem de erro detalhada do ESP32:', errorText);
      mostrarMensagemStatus(`Erro ao enviar dados: ${errorText}`, 'error');
    }
  } catch (error) {
    console.error('Erro de rede ao comunicar com o ESP32:', error);
    mostrarMensagemStatus('Erro de rede ao comunicar com o ESP32. Verifique a conexão.', 'error');
  }
}

// Função para mostrar mensagens de status na UI
function mostrarMensagemStatus(mensagem, tipo = 'info') {
  const statusMessageDiv = document.getElementById('statusMessage');
  statusMessageDiv.textContent = mensagem;
  statusMessageDiv.classList.remove('hidden', 'success', 'error', 'info'); // Limpa classes anteriores
  statusMessageDiv.classList.add(tipo); // Adiciona a classe de tipo (success, error, info)
  statusMessageDiv.style.display = 'block'; // Garante que esteja visível

  // Esconde a mensagem após alguns segundos
  // Alterado de 3000ms (3 segundos) para 5000ms (5 segundos)
  setTimeout(() => {
    statusMessageDiv.classList.add('hidden');
    statusMessageDiv.style.display = 'none';
  }, 10000); // Mensagem visível por 5 segundos
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
    mostrarMensagemStatus('Planta já adicionada!', 'info');
    return;
  }
  if (favoritas.length >= 2) {
    mostrarMensagemStatus('Você pode adicionar no máximo 2 plantas favoritas.', 'info');
    return;
  }
  favoritas.push(planta);
  atualizarFavoritas();
  // CHAMA A NOVA FUNÇÃO AQUI:
  enviarDadosParaESP32(planta); // Envia os dados da planta selecionada para o ESP32
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
