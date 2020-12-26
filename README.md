# HTQL - the Hyper-Text Query Language

The Hyper-Text Query Language (HTQL) is a language for the querying and transformation of HTML, XML and plain text documents. HTQL is developed in C++ with fast and efficient data extraction algorithms. HTQL can be used to:

- Extract data from HTML pages
- Retrieve HTML page through HTTP protocol
- Modify HTML pages

## Python Installation

Run: 
```
python setup.py install
```
  
## Python Examples

A simple example to extract url and text from links.

```
import htql

page="<a href=a.html>1</a><a href=b.html>2</a><a href=c.html>3</a>"
query="<a>:href,tx"

for url, text in htql.query(page, query): 
    print(url, text)
```

An example to parse state and zip from US address using HTQL regular expression:

```
import htql; 
address = '88-21 64th st , Rego Park , New York 11374'
states=['Alabama', 'Alaska', 'Arizona', 'Arkansas', 'California', 'Colorado', 'Connecticut', 
	'Delaware', 'District Of Columbia', 'Florida', 'Georgia', 'Hawaii', 'Idaho', 'Illinois', 'Indiana', 
	'Iowa', 'Kansas', 'Kentucky', 'Louisiana', 'Maine', 'Maryland', 'Massachusetts', 'Michigan', 
	'Minnesota', 'Mississippi', 'Missouri', 'Montana', 'Nebraska', 'Nevada', 'New Hampshire', 
	'New Jersey', 'New Mexico', 'New York', 'North Carolina', 'North Dakota', 'Ohio', 'Oklahoma', 
	'Oregon', 'PALAU', 'Pennsylvania', 'PUERTO RICO', 'Rhode Island', 'South Carolina', 'South Dakota', 
	'Tennessee', 'Texas', 'Utah', 'Vermont', 'Virginia', 'Washington', 'West Virginia', 'Wisconsin', 
	'Wyoming']; 

a=htql.RegEx(); 
a.setNameSet('states', states);

state_zip1=a.reSearchStr(address, "&[s:states][,\s]+\d{5}", case=False)[0]; 
# state_zip1 = 'New York 11374'

state_zip2=a.reSearchList(address.split(), r"&[ws:states]<,>?<\d{5}>", case=False)[0]; 
# state_zip2 = ['New', 'York', '11374']
```

## Manuals

- [Hyper-Text Query Language (HTQL)](http://htql.net/htql-manual.pdf)
- [HTQL Python Interface](http://htql.net/htql-python-manual.pdf)

## Home

  [http://htql.net](http://htql.net)

## Citation
- Liangyou Chen. `Ad Hoc Integration and Querying of Heterogeneous Online Distributed Databases`. 2004. Ph.D. Dissertation, Department of Computer Science & Engineering, Mississippi State University.
