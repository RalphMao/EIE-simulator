
from jinja2 import Template 
from lxml import html

template='''
cache_size={{ cache_size }}&nrbanks={{banks}}&rwports=0&read_ports=1&write_ports=1&ser_ports=0&output={{bits_out}}&technode=35&temp=300&
data_arr_ram_cell_tech_flavor_in=0&data_arr_periph_global_tech_flavor_in=0&tag_arr_ram_cell_tech_flavor_in=0&tag_arr_periph_global_tech_flavor_in=0&interconnect_projection_type_in=1&wire_outside_mat_type_in=0&
preview-article=Submit&action=submit&cacti=cache&pure_sram=pure_sram"
'''
jtem = Template(template)

url = "http://quid.hpl.hp.com:9081/cacti/sram.y"
def get_params(**kwargs): #(cache_size = 16384, banks = 1, bits_out = 128):
    query = jtem.render(kwargs)

    os.system("curl -sd %s %s > tmp.html"%(query, url))

    html_f = open('tmp.html').read()
    tree = html.parse(html_f)

    x = tree.xpath(r"/html/body/div[@class='page']/div[@class='contentcolumn']/table/tr[3]/td/text()")

    return x
