import distutils.core as dc

dc.setup ( 
    ext_modules = [
        dc.Extension('versus', sources = ['versus.c'])
    ])
