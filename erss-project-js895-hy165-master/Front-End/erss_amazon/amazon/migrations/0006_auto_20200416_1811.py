# Generated by Django 3.0.4 on 2020-04-16 18:11

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('amazon', '0005_orders_status'),
    ]

    operations = [
        migrations.CreateModel(
            name='Department',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('name', models.CharField(max_length=20)),
            ],
        ),
        migrations.RemoveField(
            model_name='products',
            name='department',
        ),
        migrations.AddField(
            model_name='products',
            name='department',
            field=models.ManyToManyField(to='amazon.Department'),
        ),
    ]
