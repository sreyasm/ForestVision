import boto3

# boto3 is the AWS SDK library for Python.
# The "resources" interface allow for a higher-level abstraction than the low-level client interface.
# More details here: http://boto3.readthedocs.io/en/latest/guide/resources.html
dynamodb = boto3.resource('dynamodb', region_name='us-east-2')
table = dynamodb.Table('Cameras')


# The BatchWriteItem API allows us to write multiple items to a table in one request.
with table.batch_writer() as batch:
    batch.put_item(Item={"Group": "FV", "CameraID": "1",
        "Status": "Alive", "Fire Status": "None", "Battery Life": 92 })
    batch.put_item(Item={"Group": "FV", "CameraID": "2",
        "Status": "Dead", "Fire Status": "FIRE", "Battery Life": 40 })
    batch.put_item(Item={"Group": "FV", "CameraID": "3",
        "Status": "50/50", "Fire Status": "None", "Battery Life": 76 })
    batch.put_item(Item={"Group": "FV", "CameraID": "4",
        "Status": "Hungry", "Fire Status": "FIRE", "Battery Life": 32 })
