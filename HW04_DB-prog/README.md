# Database Programming

## Background

Please see [homework4-requirement.pdf](https://github.com/menyf/ECE650/blob/master/HW04_DB-prog/requirement.pdf) for specific requirement. 


## Usage and Result

Tests were ran on Duke virtual machine with Ubuntu 18.

commit id: e82e643

**Prerequisite**: You need to create a blank database named `ACC_BBALL` on PostgreSQL server(the same machine, otherwise you need to change related setting). Username: `postgres`, password: `passw0rd`.

### cpp

This is a simple version based on C++.

```
cd HW04_DB-prog/cpp
make
./test
```

### orm

This is a ORM version(extra credit) based on Python3 and Django. You may need to install related package before use.

```
python3 manage.py flush
python3 manage.py migrate
python3 main.py
```

## Reference

- [PostgreSQL - C/C++ Interface](https://www.tutorialspoint.com/postgresql/postgresql_c_cpp.htm)
- [Django documentation](https://docs.djangoproject.com/en/2.1/)
- [Django 数据导入](https://code.ziqiangxuetang.com/django/django-import-data.html)
- [Django中ORM介绍和字段及字段参数](https://www.cnblogs.com/allen-w/p/9209315.html)
- [Django ORM操作及进阶](https://www.cnblogs.com/study-learning/p/9969486.html)