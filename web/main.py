#! /usr/bin/env python3.6

import flask
import time
import datetime
import database as db

app = flask.Flask(__name__)

js_time1 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_time2 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_time3 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_time4 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

js_ftime1 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_ftime2 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_ftime3 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000
js_ftime4 = int(time.mktime(datetime.datetime.utcnow().timetuple())) * 1000

def update_camera_page():

    with open("templates/cameras.html", "r") as infile:
        html_contents = infile.read()

    with open("templates/cameras.html", "w") as outfile:
        outfile.write(html_contents)

    return

def update_camera(parsed_text):
    
    # Format is (CameraID, Status, Fire Status, Battery Life)
    db.update_item(parsed_text[0], parsed_text[1], parsed_text[2], int(parsed_text[3]))
    update_time(int(parsed_text[0]))

    if parsed_text[2].lower() == 'fire'.lower():
        update_fire_time(int(parsed_text[0]))

    return

def parse_text(text):
    text = text.strip()
    list_of_nodes = text.split(".")
    successfully_updated_nodes = []
    
    for node in list_of_nodes:
        node = node.strip()
        node_tokens_unstripped = node.split(",")
        node_tokens = []

        for item in node_tokens_unstripped:
            node_tokens.append(item.strip())
        
        try:
            int(node_tokens[0])
        except ValueError:
            continue
        else:
            node_tokens.insert(1, "Alive")
            update_camera(node_tokens)
            successfully_updated_nodes.append(int(node_tokens[0]))

    for i in range(1, 4+1):
        if i not in successfully_updated_nodes:
            # At this point, the node in question's update did not come in. We assume it's dead
            update = [str(i), "Dead", "N/A", 0]
            update_camera(update)


def process(input_text):
    parse_text(input_text)
    update_camera_page()

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


# @app.route('/')
# def index_page():
#     return flask.render_template('index.html', value1 = count)


@app.route('/')
def cameras_page():
    items = db.get_all_items()
    return flask.render_template('cameras.html', s_time1 = js_time1, s_time2 = js_time2, s_time3 = js_time3, s_time4 = js_time4, s_ftime1 = js_ftime1, s_ftime2 = js_ftime2, s_ftime3 = js_ftime3, s_ftime4 = js_ftime4, cam_id1 = items[0]['CameraID'], status1 = items[0]['Status'], fstat1 = items[0]['Fire Status'], battery1 = items[0]['Battery Life'], cam_id2 = items[1]['CameraID'], status2 = items[1]['Status'], fstat2 = items[1]['Fire Status'], battery2 = items[1]['Battery Life'], cam_id3 = items[2]['CameraID'], status3 = items[2]['Status'], fstat3 = items[2]['Fire Status'], battery3 = items[2]['Battery Life'], cam_id4 = items[3]['CameraID'], status4 = items[3]['Status'], fstat4 = items[3]['Fire Status'], battery4 = items[3]['Battery Life'])


@app.route('/post', methods=['POST'])
def accept():
    text = flask.request.form['Body']
    process(text)
    return xml_response()


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, use_reloader=True, use_evalex=False, debug=True, use_debugger=False)
