from django import forms
from django.forms import ModelForm
from django.contrib.auth.models import User
from django.contrib.auth.forms import UserCreationForm
from .models import AmazonUser, Department, Products, Orders
from django.utils.safestring import mark_safe


class RegisterForm(UserCreationForm):
    ups_username = forms.CharField(max_length=100)
    email = forms.EmailField()

    class Meta:
        model = AmazonUser
        fields = ['username', 'email', 'password1', 'password2', 'ups_username']


class CatalogueForm(forms.ModelForm):

    class Meta:
        model = Products
        fields = ['name', 'department', 'price']
        labels = {
            "name": "Product Name",
            "department": "Department",
            "price": "Price",
        }
        widgets = {
            "name": forms.widgets.TextInput(attrs={"class": "form-control"}),
            "department": forms.widgets.TextInput(attrs={"class": "form-control"}),
            "price": forms.widgets.TextInput(attrs={"class": "form-control", "step":"0.01"}),
        }

class PlaceOrderForm(forms.ModelForm):

    description = forms.ModelChoiceField(queryset=Products.objects.all())

    class Meta:
        model = Orders
        fields = ['description', 'count', 'ship_addr_x', 'ship_addr_y']
        labels = {
            "description": "Product Name",
            "count": "Count",
            "ship_addr_x": "Address_x",
            "ship_addr_y": "Address_y",
        }
        widgets = {
            # "description": forms.widgets.TextInput(attrs={"class": "form-control"}),
            "count": forms.widgets.NumberInput(attrs={"class": "form-control"}),
            "ship_addr_x": forms.widgets.NumberInput(attrs={"class": "form-control"}),
            "ship_addr_y": forms.widgets.NumberInput(attrs={"class": "form-control"}),
        }


# class ViewOrdersForm(forms.ModelForm):

#     class Meta:
#         model = Orders
#         fields = ['owner', 'description', 'count', 'ship_addr_x', 'ship_addr_y', 'ship_id', 'truck_id', 'status']
#         labels = {
#             "owner": "Username",
#             "description": "Product Name",
#             "count": "Count",
#             "ship_addr_x": "Address_x",
#             "ship_addr_y": "Address_y",
#             "ship_id": "Ship id",
#             "truck_id": "Truck id",
#             "status": "Order Status",

#         }
#         widgets = {
#             "owner": forms.widgets.TextInput(attrs={"class": "form-control"}),
#             "description": forms.widgets.TextInput(attrs={"class": "form-control"}),
#             "count": forms.widgets.NumberInput(attrs={"class": "form-control"}),
#             "ship_addr_x": forms.widgets.NumberInput(attrs={"class": "form-control"}),
#             "ship_addr_y": forms.widgets.NumberInput(attrs={"class": "form-control"}),
#             "ship_id": forms.widgets.NumberInput(attrs={"class": "form-control"}),
#             "truck_id": forms.widgets.NumberInput(attrs={"class": "form-control"}),
#             "status": forms.widgets.TextInput(attrs={"class": "form-control"}),
#         }


