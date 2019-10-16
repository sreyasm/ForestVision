#! /usr/bin/env python3.6

import flask
import time
import datetime

app = flask.Flask(__name__)

count = 0

js_time1 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_time2 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_time3 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_time4 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

def update_index_helper(new_text):

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

def update_index(input_text):
    global count

    if input_text.lower() == 'increment'.lower():
        count += count
    elif input_text.lower() == 'decrement'.lower():
        count -= count
    else:
        update_index_helper(input_text)

def update_cameras(parsed_text):

    new_html = ""

    with open("templates/status.html", "r") as infile:
        html_contents = infile.read()

    for line in html_contents.splitlines():
        if '<td id="c{}">'.format(parsed_text[0]) in line:
            line = line.replace(line, '<td id="c{}"> {} </td>'.format(parsed_text[0], parsed_text[1]))

        new_html += line + '\n'

    with open("templates/status.html", "w") as outfile:
        outfile.write(new_html)

    update_time(int(parsed_text[0]))

    return

def parse_text(text):
    text = text.strip()
    ori_list = text.split(",")
    strip_list = []
    for item in ori_list:
        strip_list.append(item.strip())

    return strip_list

def process(input_text):
    processed_text = parse_text(input_text)

    try:
        int(processed_text[0])
    except ValueError:
        update_index(input_text)
    else:
        update_cameras(processed_text)

def xml_response():
    xml = '<Response></Response>'
    return flask.Response(xml, mimetype='text/xml')

def update_time(num):
    global js_time1
    global js_time2
    global js_time3
    global js_time4

    if (num == 1):
        js_time1 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

    elif (num == 2):
        js_time2 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

    elif (num == 3):
        js_time3 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

    elif (num == 4):
        js_time4 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

    return

@app.route('/')
def index_page():
    return flask.render_template('index.html', value1 = count)

@app.route('/cameras')
def cameras_page():
    return flask.render_template('cameras.html', s_time1 = js_time1, s_time2 = js_time2, s_time3 = js_time3, s_time4 = js_time4)

@app.route('/test')
def test_page():
    return flask.render_template('test.php')

@app.route('/post', methods=['POST'])
def accept():
    text = flask.request.form['Body']
    process(text)
    return xml_response()

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, use_reloader=True, use_evalex=False, debug=True, use_debugger=False)
