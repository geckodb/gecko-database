# Table of Contents

`Copy auto generated TOC here`



REST API requests are sent to the URL `${GRID_STORE_URL}`. This URL is where the internal http-server of GridStore is accessible. When running on a developer machine, the http-server is listening on `${GRID_STORE_URL}=
http://localhost:36895`.

[TOC]

# REST API Data Types

## String

A string is list of characters. When there are no whitespaces in the string, no quotes are required. For instance, `movie`. In contrast, when whitespaces are contained, quotes are required, e.g., `"that movie"`. 

## List

A list is a unorder collection of elements formatted as **YAML** *inline list*. For example, `[a, b, c]`.

## Map

A map is a **YAML** formated *inline list* that associates keys to values. For example `{name: char(256), age: u8}` describes a list of two elements, `name` that maps to `char(256)` and `age` that maps to `u8`. A map is used for instance to define the attributes for a certain type, see [Create Type](#create-type) command.

## Attribute Types

A attribute type defines a certain data types for an attribute for a user-defined type in GridStore. GridStore supports the following attribute types:

| Name      |  Bytes | Description
| --------- | ------ | -----------
| `char`    |  1     | 
| `bool`    |  1     |
| `s8`      |  1     |
| `u8`      |  1     |
| `s16`     |  2     |
| `u16`     |  2     |
| `s32`     |  4     |
| `u32`     |  4     |
| `float32` |  4     |
| `s64`     |  8     |
| `u64`     |  8     |
| `float64` |  8     |




# Data Definition 

## Type Objects

### Type Creation

#### Create Type

Creates a type in the current database.

##### Request

> **`PUT`** `${GRID_STORE_URL}/api/types/create`

###### Body Params

| Key          | Type               | Value                                                | Comment       |
| ------------ | ------------------ | ---------------------------------------------------- | ------------- |
| `name`       | [string](#string)  | The (unique) type name to be created.                | **required**  |
| `attributes` | [map](#map)        | A list of `name` to `type` mappings. Each `name` is a [string](#string). Each `type` belongs to grid store's [attribute types](#attriubte-types). | **required**  |
| `primary`    | [list](#list)      | A list of name to type mappings for the type `name`. | **required**  |

###### Headers

No further information in the header is required.

###### Sample

````bash

curl http://localhost:36895/api/types/create \
  -X PUT \
  -F name="Movie" \
  -F attributes='{name: char(256), released: u64, rating: u8}' \
  -F primary='[name]'

````

##### Response

A status code `201` will be returned in case of success, otherwise `406`.

#### Comments

The request will be rejected when one of the following conditions are violated:

- The key `name` must be an unique type name w.r.t. the type names already registered to the system.
- The key `attributes` must be non-empty, and each attribute `name` must be unique in scope of the list
- The key `primary` must be non-empty, and must contain at least one attribute name defined in the `attributes` key







### Node Creation

#### Create Node

Creates a new node in the current database.

##### Request

> **`POST`** `${GRID_STORE_URL}/api/1.0/nodes`

###### Body Params

| Key    | Type       | Value                | Comment       |
| ------ | ---------- | -------------------- | ------------- |
| `name` | string     | The name of the type | **required**  |

###### Headers

No further information in the header is required.

###### Sample

````bash

curl http://localhost:35497/api/1.0/nodes -X POST \
  -F nodes='{[{"name":{"type":"string", value="mike"}, "age":{"type":"u8", value="32"}}]}' 

````

##### Response

A `201` will be returned in case of success, otherwise `http-code`.

#### Comments

- Some comments and restrictions





# Data Definition 

## Type Objects

### Type Creation

#### Create Type

Creates a type in the current database.

##### Request

> **`PUT`** `${GRID_STORE_URL}/api/1.0/types/catalog`

###### Body Params

| Key    | Type       | Value                | Comment       |
| ------ | ---------- | -------------------- | ------------- |
| `name` | string     | The name of the type | **required**  |

###### Headers

No further information in the header is required.

###### Sample

````bash

curl https://upload.box.com/api/2.0/files/content \
  -H "Authorization: Bearer ACCESS_TOKEN" -X POST \
  -F attributes='{"name":"tigers.jpeg", "parent":{"id":"11446498"}}' \
  -F file=@myfile.jpg

````

##### Response

A `201` will be returned in case of success, otherwise `http-code`.

#### Comments

- Some comments and restrictions