#!/usr/bin/env python3
"""Generate a large JSON file for testing streaming capabilities."""
import json
import random
import string

def generate_random_string(length=20):
    return ''.join(random.choices(string.ascii_letters + string.digits, k=length))

def generate_record(i):
    return {
        "id": i,
        "uuid": generate_random_string(32),
        "name": f"User_{i}",
        "email": f"user_{i}@example.com",
        "active": random.choice([True, False]),
        "score": round(random.uniform(0, 100), 2),
        "tags": random.sample(["a", "b", "c", "d", "e", "f", "g", "h"], k=random.randint(1, 4)),
        "metadata": {
            "created": f"2024-{random.randint(1, 12):02d}-{random.randint(1, 28):02d}",
            "updated": f"2024-{random.randint(1, 12):02d}-{random.randint(1, 28):02d}",
            "version": random.randint(1, 10)
        }
    }

def main():
    # Generate 1000 records
    records = [generate_record(i) for i in range(1, 1001)]
    
    # Write as JSON array
    with open("large_array.json", "w") as f:
        json.dump(records, f, indent=2)
    
    # Write as JSONL
    with open("large.jsonl", "w") as f:
        for record in records:
            f.write(json.dumps(record) + "\n")
    
    print("Generated:")
    print("  - large_array.json (JSON array with 1000 records)")
    print("  - large.jsonl (JSON Lines with 1000 records)")

if __name__ == "__main__":
    main()
