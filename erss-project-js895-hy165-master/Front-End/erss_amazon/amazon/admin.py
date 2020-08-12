from django.contrib import admin

from .models import AmazonUser, Department, Products, Orders


admin.site.register(AmazonUser)
admin.site.register(Department)
admin.site.register(Products)
admin.site.register(Orders)