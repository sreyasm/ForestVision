#! /usr/bin/env python3.6

import boto3
from boto3.dynamodb.conditions import Key

dynamodb = boto3.resource('dynamodb', region_name='us-east-1')
table = dynamodb.Table('Cameras')

def get_item(cam_id):
    resp = table.get_item(Key={"Group": "FV", "CameraID": cam_id})
    return resp['Item'] # Returns an item, a dict containing the attributes

def get_all_items():
    resp = table.query(KeyConditionExpression=Key('Group').eq('FV'))
    return resp['Items'] # Returns list of items, use loop to iterate through

def insert_item(cam_id, status, battery_life):
    with table.batch_writer() as batch:
        batch.put_item(Item={"Group": "FV", "CameraID": cam_id,
            "Status": status, "Battery Life": battery_life})

def update_item(cam_id, status, battery_life):
    resp = table.update_item(
        Key={"Group": "FV", "CameraID": cam_id},
        ExpressionAttributeNames={
            "#status": "Status",
            "#battery_life": "Battery Life",
        },
        ExpressionAttributeValues={
            ":s": status,
            ":b": battery_life,
        },
        UpdateExpression="SET #status = :s, #battery_life = :b",
    )

if __name__ == '__main__':
    
    for item in get_all_items():
        print(item)
