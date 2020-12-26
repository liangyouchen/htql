# HTQL - the Hyper-Text Query Language

Hyper-Text Query Language (HTQL) is a language for the querying and transformation of HTML, XML and plain text documents. HTQL is developed in C++ with fast and efficient data extraction algorithms. HTQL can be used to:

- Extract data from HTML pages
- Retrieve HTML page through HTTP protocol
- Modify HTML pages

## How to install

Run: 
> python setup.py install</code>

## Homepage

  http://htql.net
  
## Examples

A simple example to extract url and text from links.

```
import htql

page="<a href=a.html>1</a><a href=b.html>2</a><a href=c.html>3</a>"
query="<a>:href,tx"

for url, text in htql.query(page, query): 
    print(url, text)
```

## Manual

  http://htql.net/htql-manual.pdf
  
## Citation
- Liangyou Chen. `Ad Hoc Integration and Querying of Heterogeneous Online Distributed Databases`. 2004. Ph.D. Dissertation, Department of Computer Science & Engineering, Mississippi State University.
