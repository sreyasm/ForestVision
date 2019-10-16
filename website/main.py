#! /usr/bin/env python3.6

import flask
import time
import datetime
import database as db

app = flask.Flask(__name__)

count = 0

js_time1 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_time2 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_time3 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_time4 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

js_ftime1 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_ftime2 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_ftime3 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_ftime4 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

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

def update_camera_page():

    with open("templates/cameras.html", "r") as infile:
        html_contents = infile.read()

    with open("templates/cameras.html", "w") as outfile:
        outfile.write(html_contents)

    return

def update_cameras(parsed_text):

    db.update_item(parsed_text[0], parsed_text[1], int(parsed_text[2]))
    update_time(int(parsed_text[0]))

    if parsed_text[1].lower() == 'fire'.lower():
        update_fire_time(int(parsed_text[0]))

    update_camera_page()

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

def update_fire_time(num):
    global js_ftime1
    global js_ftime2
    global js_ftime3
    global js_ftime4

    if (num == 1):
        js_ftime1 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

    elif (num == 2):
        js_ftime2 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

    elif (num == 3):
        js_ftime3 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

    elif (num == 4):
        js_ftime4 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

    return

@app.route('/')
def index_page():
    return flask.render_template('index.html', value1 = count)

@app.route('/cameras')
def cameras_page():
    items = db.get_all_items()
    return flask.render_template('cameras.html', s_time1 = js_time1, s_time2 = js_time2, s_time3 = js_time3, s_time4 = js_time4, s_ftime1 = js_ftime1, s_ftime2 = js_ftime2, s_ftime3 = js_ftime3, s_ftime4 = js_ftime4, cam_id1 = items[0]['CameraID'], status1 = items[0]['Status'], battery1 = items[0]['Battery Life'], cam_id2 = items[1]['CameraID'], status2 = items[1]['Status'], battery2 = items[1]['Battery Life'], cam_id3 = items[2]['CameraID'], status3 = items[2]['Status'], battery3 = items[2]['Battery Life'], cam_id4 = items[3]['CameraID'], status4 = items[3]['Status'], battery4 = items[3]['Battery Life'])

@app.route('/post', methods=['POST'])
def accept():
    text = flask.request.form['Body']
    process(text)
    return xml_response()

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, use_reloader=True, use_evalex=False, debug=True, use_debugger=False)
