#! /usr/bin/env python3.6

import flask
import flask_socketio

app = flask.Flask(__name__)
socketio = flask_socketio.SocketIO(app)

count = 0

def change_text(new_text):

    new_html = ""

    with open("templates/index.html", "r") as infile:
        html_contents = infile.read()

    for line in html_contents.splitlines():
        if '<h2>' in line:
            line = line.replace(line, '<h2> {} </h2>'.format(new_text))

        new_html += line + '\n'

    with open("templates/index.html", "w") as outfile:
        outfile.write(new_html)

    return

def process(input_text):
    global count

    if input_text.lower() == 'increment'.lower():
        count = count + 1
    elif input_text.lower() == 'decrement'.lower():
        count = count - 1
    else:
        change_text(input_text)

def xml_response():
    xml = '<Response></Response>'
    return flask.Response(xml, mimetype='text/xml')

@app.route('/')
def index_page():
    return flask.render_template('index.html', value1 = count)

@app.route('/post', methods=['POST'])
def accept():
    text = flask.request.form['Body']
    process(text)
    return xml_response()

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, use_reloader=True, use_evalex=False, debug=True, use_debugger=False)
    # socketio.run(app, host='0.0.0.0' port=8080, debug=True, use_debugger=False)
