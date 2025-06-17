from flask import Flask, render_template, request, jsonify
from flask_cors import CORS # Import CORS

# Cria a aplicação Flask
app = Flask(__name__)
CORS(app) # Habilita CORS para todas as rotas

# Variável global para armazenar a informação mais recente
# Começa com uma mensagem padrão
informacao_para_esp = "Aguardando informacao..."

# Rota principal: mostra a página web (o site)
@app.route('/')
def index():
    # Renderiza o arquivo 'index.html' que está na pasta 'templates'
    return render_template('index.html')

# Rota para ATUALIZAR a informação (usada pelo site)
# Aceita requisições do tipo POST
@app.route('/atualizar', methods=['POST'])
def atualizar_informacao():
    global informacao_para_esp
    # Pega o dado JSON enviado pela página web
    dados = request.get_json()
    if dados and 'nova_informacao' in dados:
        informacao_para_esp = dados['nova_informacao']
        print(f"Informacao atualizada para: '{informacao_para_esp}'")
        # Responde à página web que tudo correu bem
        return jsonify(status="sucesso", info=informacao_para_esp)
    return jsonify(status="erro", mensagem="Dados invalidos"), 400

# Rota para o ESP32 PEGAR a informação
# Aceita requisições do tipo GET
@app.route('/dados')
def obter_dados_para_esp():
    # Simplesmente retorna a informação armazenada em texto plano
    return informacao_para_esp

# NOVA ROTA: Para receber dados da planta do script.js
@app.route('/selectPlant', methods=['POST'])
def select_plant():
    dados_planta = request.get_json()
    if dados_planta:
        print(f"Dados da planta recebidos do frontend: {dados_planta}")
        global informacao_para_esp
        if 'nome_popular' in dados_planta and 'umidade_solo' in dados_planta and 'uv_dia' in dados_planta and 'horas_sol_pleno_dia' in dados_planta:
            # Formate a informação para o ESP32 como uma string simples
            # Por exemplo: "Rosa;40,60;6,8"
            umidade = str(dados_planta['umidade_solo']).replace(' ', '')
            uv = str(dados_planta['uv_dia']).replace(' ', '')
            tempo = str(dados_planta['horas_sol_pleno_dia']).replace(' ','')
            informacao_para_esp = f"{dados_planta['nome_popular']};{umidade};{uv};{tempo}"
            print(f"Informacao para ESP atualizada para: '{informacao_para_esp}'")

        return jsonify(status="sucesso", mensagem="Dados da planta recebidos com sucesso!"), 200
    return jsonify(status="erro", mensagem="Nenhum dado de planta recebido."), 400

# Função principal para rodar o servidor
if __name__ == '__main__':
    # '0.0.0.0' faz o servidor ser acessível por outros dispositivos na mesma rede
    # (como o seu ESP32)
    app.run(host='0.0.0.0', port=5000, debug=True)