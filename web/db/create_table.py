import boto3

# boto3 is the AWS SDK library for Python.
# We can use the low-level client to make API calls to DynamoDB.
client = boto3.client('dynamodb', region_name='us-east-2')

try:
    resp = client.create_table(
        TableName="Cameras",
        # Declare your Primary Key in the KeySchema argument
        KeySchema=[
            {
                "AttributeName": "Group",
                "KeyType": "HASH"
            },
            {
                "AttributeName": "CameraID",
                "KeyType": "RANGE"
            }
        ],
        # Any attributes used in KeySchema or Indexes must be declared in AttributeDefinitions
        AttributeDefinitions=[
            {
                "AttributeName": "Group",
                "AttributeType": "S"
            },
            {
                "AttributeName": "CameraID",
                "AttributeType": "S"
            }
        ],
        # ProvisionedThroughput controls the amount of data you can read or write to DynamoDB per second.
        # You can control read and write capacity independently.
        ProvisionedThroughput={
            "ReadCapacityUnits": 20,
            "WriteCapacityUnits": 20
        }
    )
    print("Table created successfully!")

except Exception as e:
    print("Error creating table:")
    print(e)

