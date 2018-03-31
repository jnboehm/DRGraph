from django.conf.urls import url
from views import *

urlpatterns = [
    url(r'^getDatasets/$', getDatasets, name='getDatasets'),
    url(r'^runGraphLayout/$', runGraphLayout, name='runGraphLayout'),
    url(r'^upload/$', upload, name='upload')
]
