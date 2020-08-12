# Generated by Django 3.0.4 on 2020-04-14 23:28

from django.conf import settings
from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('amazon', '0001_initial'),
    ]

    operations = [
        migrations.CreateModel(
            name='Products',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('description', models.CharField(max_length=300)),
                ('department', models.CharField(choices=[('Food', 'Food'), ('Fashion', 'Fashion'), ('Household', 'Household'), ('Sports', 'Sports'), ('Others', 'Others')], default='Others', max_length=100)),
                ('price', models.DecimalField(decimal_places=2, max_digits=10)),
                ('wh_id', models.IntegerField(blank=True)),
            ],
        ),
        migrations.AddField(
            model_name='amazonuser',
            name='ups_username',
            field=models.CharField(default='xxx', max_length=100),
        ),
        migrations.CreateModel(
            name='Orders',
            fields=[
                ('id', models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('ship_addr', models.CharField(max_length=300)),
                ('ship_id', models.IntegerField(blank=True)),
                ('track_num', models.CharField(blank=True, max_length=50)),
                ('owner', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, to=settings.AUTH_USER_MODEL)),
            ],
        ),
    ]
