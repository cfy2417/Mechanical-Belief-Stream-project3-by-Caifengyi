from flask import Flask, request, jsonify, send_file
import os

app = Flask(__name__)

# 修改成你在 TouchDesigner 中保存 JPEG 的路径
image_path = "/Users/caifengyi/Desktop/project3photo/image.jpg"

@app.route('/image.jpg')
def image():
    if os.path.exists(image_path):
        return send_file(image_path, mimetype='image/jpg')
    else:
        return "Image not found", 404
    
stored_data = {}

@app.route('/data', methods=['GET', 'POST'])
def data():
    global stored_data
    if request.method == 'POST':
        content = request.get_json()
        if content:
            stored_data = content
            print("Received data:", stored_data)
            return jsonify({"status": "success"})
        else:
            return jsonify({"status": "fail", "reason": "No JSON received"}), 400
    else:  # GET
        return jsonify(stored_data)


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5050)  # 监听整个局域网
