import boto3

# boto3 is the AWS SDK library for Python.
# The "resources" interface allow for a higher-level abstraction than the low-level client interface.
# More details here: http://boto3.readthedocs.io/en/latest/guide/resources.html
dynamodb = boto3.resource('dynamodb', region_name='us-east-2')
table = dynamodb.Table('Cameras')

# When making a GetItem call, we specify the Primary Key attributes defined on our table for the desired item.
resp = table.get_item(Key={"Group": "FV", "CameraID": "1"})

print(resp['Item'])
print(resp['Item']['Status'])
